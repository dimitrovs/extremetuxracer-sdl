/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "winsys.h"
#include "ogl.h"
#include "audio.h"
#include "game_ctrl.h"
#include "font.h"
#include "score.h"
#include "textures.h"
#include "spx.h"
#include "course.h"
#include <iostream>

#define USE_JOYSTICK true

TVector2i cursor_pos(0, 0);

CWinsys Winsys;

CWinsys::CWinsys()
	: auto_resolution(800, 600) {
	window = NULL;
	key_repeat = false;
	glcontext = NULL;
	controller = NULL;

	joystick = NULL;
	numJoysticks = 0;
	joystick_active = false;

	resolutions[0] = TScreenRes(0, 0);
	resolutions[1] = TScreenRes(800, 600);
	resolutions[2] = TScreenRes(1024, 768);
	resolutions[3] = TScreenRes(1152, 864);
	resolutions[4] = TScreenRes(1280, 960);
	resolutions[5] = TScreenRes(1280, 1024);
	resolutions[6] = TScreenRes(1360, 768);
	resolutions[7] = TScreenRes(1400, 1050);
	resolutions[8] = TScreenRes(1440, 900);
	resolutions[9] = TScreenRes(1680, 1050);
}

const TScreenRes& CWinsys::GetResolution(size_t idx) const {
	if (idx >= NUM_RESOLUTIONS || (idx == 0 && !param.fullscreen)) return auto_resolution;
	return resolutions[idx];
}

string CWinsys::GetResName(size_t idx) const {
	if (idx >= NUM_RESOLUTIONS) return "800 x 600";
	if (idx == 0) return ("auto");
	string line = Int_StrN(resolutions[idx].width);
	line += " x " + Int_StrN(resolutions[idx].height);
	return line;
}

float CWinsys::CalcScreenScale() const {
	if (resolution.height < 768) return 0.78f;
	else return (resolution.height / 768.f);
}

extern "C" {
#include <gl4esinit.h>
}

static void* etr_gl_getproc(const char* name) {
	return SDL_GL_GetProcAddress(name);
}

static void etr_gl_fbsize(int* w, int* h) {
	*w = Winsys.resolution.width;
	*h = Winsys.resolution.height;
}

void CWinsys::SetupVideoMode(const TScreenRes& resolution_) {
	if (window == NULL) {
		// The handheld panel has one fixed mode: always go fullscreen at native size.
		SDL_DisplayMode dm;
		int w = resolution_.width, h = resolution_.height;
		if (SDL_GetCurrentDisplayMode(0, &dm) == 0 && dm.w > 0 && dm.h > 0) {
			w = dm.w;
			h = dm.h;
		}
		if (w <= 0 || h <= 0) { w = 1280; h = 720; }

		window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		                          w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
		if (window == NULL) {
			Message("couldn't create window", SDL_GetError());
			exit(1);
		}
		glcontext = SDL_GL_CreateContext(window);
		if (glcontext == NULL) {
			Message("couldn't create GL context", SDL_GetError());
			exit(1);
		}
		SDL_GL_MakeCurrent(window, glcontext);
		SDL_GL_SetSwapInterval(1);
		SDL_GL_GetDrawableSize(window, &w, &h);
		resolution.width = w;
		resolution.height = h;
		auto_resolution = resolution;

		set_getprocaddress(etr_gl_getproc);
		set_getmainfbsize(etr_gl_fbsize);
		initialize_gl4es();
	}
	scale = CalcScreenScale();
	if (param.use_quad_scale) scale = sqrt(scale);
}

void CWinsys::SetupVideoMode(size_t idx) {
	SetupVideoMode(GetResolution(idx));
}

void CWinsys::SetupVideoMode(int width, int height) {
	SetupVideoMode(TScreenRes(width, height));
}

void CWinsys::InitJoystick() {
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
		Message("Could not initialize SDL_joystick: %s", SDL_GetError());
		return;
	}
	numJoysticks = SDL_NumJoysticks();
	if (numJoysticks < 1) {
		joystick = NULL;
		return;
	}
	SDL_JoystickEventState(SDL_ENABLE);
	joystick = SDL_JoystickOpen(0);	// first stick with number 0
	if (joystick == NULL) {
		Message("Cannot open joystick %s", SDL_GetError());
		return;
	}
	joystick_active = true;
	if (SDL_IsGameController(0)) controller = SDL_GameControllerOpen(0);
	Message("Joystick: ", SDL_JoystickName(joystick));
	Message(controller ? "using SDL game controller mapping" : "using raw joystick mapping");
}

void CWinsys::Init() {
	Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER;
	if (SDL_Init(sdl_flags) < 0) Message("Could not initialize SDL", SDL_GetError());
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#if defined (USE_STENCIL_BUFFER)
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif

	SetupVideoMode(GetResolution(param.res_type));
	Reshape(resolution.width, resolution.height);

	if (USE_JOYSTICK) InitJoystick();
//	SDL_EnableUNICODE (1);
}

void CWinsys::KeyRepeat(bool repeat) {
	key_repeat = repeat;
}

void CWinsys::CloseJoystick() {
	if (controller) SDL_GameControllerClose(controller);
	else if (joystick_active) SDL_JoystickClose(joystick);
	controller = NULL;
	joystick_active = false;
}

void CWinsys::Quit() {
	CloseJoystick();
	Score.SaveHighScore();
	SaveMessages();
	Audio.Close();		// frees music and sound as well
	if (g_game.argument < 1) Players.SavePlayers();
	SDL_Quit();
	// Trimui port: the GL context is gone now, so skip the static destructors
	// that would still try to delete textures through it (crash on exit).
	fflush(NULL);
	_exit(0);
}

void CWinsys::Terminate() {
	Quit();
	exit(0);
}

void CWinsys::PrintJoystickInfo() const {
	if (joystick_active == false) {
		Message("No joystick found");
		return;
	}
	PrintStr("");
	PrintStr(SDL_JoystickName(joystick));
	int num_buttons = SDL_JoystickNumButtons(joystick);
	cout << "Joystick has " << num_buttons << " button" << (num_buttons == 1 ? "" : "s") << '\n';
	int num_axes = SDL_JoystickNumAxes(joystick);
	cout << "Joystick has " << num_axes << " ax" << (num_axes == 1 ? "i" : "e") << "s\n\n";
}

