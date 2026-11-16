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

#include "states.h"
#include "ogl.h"
#include "winsys.h"
#include <ctime>
#include <cstring>
#include "racing.h"

State::Manager State::manager(Winsys);

State::Manager::~Manager() {
	if (current)
		current->Exit();
}

void State::Manager::Run(State& entranceState) {
	current = &entranceState;
	current->Enter();
	clock_t ticks = clock();
	while (!quit) {
		PollEvent();
		if (next)
			EnterNextState();
		CallLoopFunction();
		if (param.framerate != 0) {
			clock_t nticks = clock();
			int32_t sleeptime = (int32_t)(CLOCKS_PER_SEC / param.framerate - (nticks - ticks)) * (1000 / CLOCKS_PER_SEC);
			if (sleeptime > 0)
				SDL_Delay(sleeptime);
			ticks = nticks;
		}
	}
	current->Exit();
	previous = current;
	current = NULL;
}

void State::Manager::EnterNextState() {
	current->Exit();
	previous = current;
	current = next;
	next = NULL;
	current->Enter();
}

// ---- Trimui port: translate gamepad input into the keys the game expects ----

#define PAD_MAX_BUTTONS 32
static unsigned int pad_sent_key[PAD_MAX_BUTTONS];	// key sent on press, reused on release
static unsigned int stick_key[2];					// arrow key currently held by the stick

static bool InRace() {
	return State::manager.CurrentState() == &Racing;
}

static void SendKey(State* current, unsigned int key, bool release) {
	int x, y;
	SDL_GetMouseState(&x, &y);
	SDL_Keysym sym;
	memset(&sym, 0, sizeof(sym));
	sym.sym = key;
	current->Keyb(key, key >= 256, release, x, y);
	current->Keyb_spec(sym, release);
}

// logical buttons, in SDL_GameControllerButton order where possible
static unsigned int KeyForPadButton(int button) {
	bool race = InRace();
	switch (button) {
		case SDL_CONTROLLER_BUTTON_DPAD_UP: return SDLK_UP;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return SDLK_DOWN;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return SDLK_LEFT;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return SDLK_RIGHT;
		case SDL_CONTROLLER_BUTTON_B: return race ? SDLK_UP : SDLK_RETURN;		// Trimui A (east): paddle / confirm
		case SDL_CONTROLLER_BUTTON_A: return race ? SDLK_DOWN : SDLK_ESCAPE;	// Trimui B (south): brake / back
		case SDL_CONTROLLER_BUTTON_X:
		case SDL_CONTROLLER_BUTTON_Y:
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return race ? SDLK_SPACE : SDLK_RETURN;	// jump
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return race ? SDLK_r : SDLK_TAB;	// reset tux
		case SDL_CONTROLLER_BUTTON_START: return race ? SDLK_p : SDLK_RETURN;
		case SDL_CONTROLLER_BUTTON_BACK:
		case SDL_CONTROLLER_BUTTON_GUIDE: return SDLK_ESCAPE;
		default: return 0;
	}
}

static void PadButton(State* current, int button, bool pressed) {
	if (button < 0 || button >= PAD_MAX_BUTTONS) return;
	if (pressed) {
		if (pad_sent_key[button]) return;
		unsigned int key = KeyForPadButton(button);
		if (!key) return;
		pad_sent_key[button] = key;
		if (getenv("ETR_DEBUG_INPUT")) printf("pad button %d -> key %u\n", button, key);
		SendKey(current, key, false);
	} else if (pad_sent_key[button]) {
		unsigned int key = pad_sent_key[button];
		pad_sent_key[button] = 0;
		SendKey(current, key, true);
	}
}

// raw joystick button index -> logical button (Trimui "TRIMUI Player1" xpad-style layout)
static int RawToPadButton(int raw) {
	static const int map[] = {
		SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_X,
		SDL_CONTROLLER_BUTTON_Y, SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
		SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, SDL_CONTROLLER_BUTTON_BACK,
		SDL_CONTROLLER_BUTTON_START, SDL_CONTROLLER_BUTTON_GUIDE
	};
	if (raw < 0 || raw >= (int)(sizeof(map) / sizeof(map[0]))) return -1;
	return map[raw];
}

static void PadAxis(State* current, int axis, int value) {
	if (axis > 1) return;
	float val = (float)value / 32768.f;
	if (InRace()) {
		if (stick_key[axis]) { SendKey(current, stick_key[axis], true); stick_key[axis] = 0; }
		if (axis == 0) current->Jaxis(axis, val);	// analog steering only; paddling stays on buttons
		return;
	}
	unsigned int want = 0;
	if (val < -0.5f) want = (axis == 0) ? SDLK_LEFT : SDLK_UP;
	else if (val > 0.5f) want = (axis == 0) ? SDLK_RIGHT : SDLK_DOWN;
	if (want == stick_key[axis]) return;
	if (stick_key[axis]) SendKey(current, stick_key[axis], true);
	stick_key[axis] = want;
	if (want) SendKey(current, want, false);
}

void State::Manager::PollEvent() {
	SDL_Event event;
	unsigned int key;
	int x, y;
	bool has_controller = Winsys.GetController() != NULL;

	while (SDL_PollEvent(&event)) {
		if (!next) {
			switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.repeat && !Winsys.key_repeat) break;
					if (getenv("ETR_DEBUG_INPUT")) printf("keyboard key %d\n", (int)event.key.keysym.sym);
					SDL_GetMouseState(&x, &y);
					key = event.key.keysym.sym;
					current->Keyb(key, key >= 256, false, x, y);
					current->Keyb_spec(event.key.keysym, false);
					break;

				case SDL_KEYUP:
					SDL_GetMouseState(&x, &y);
					key = event.key.keysym.sym;
					current->Keyb(key, key >= 256, true, x, y);
					current->Keyb_spec(event.key.keysym, true);
					break;

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					current->Mouse(event.button.button, event.button.state, event.button.x, event.button.y);
					break;

				case SDL_MOUSEMOTION: {
					TVector2i old = cursor_pos;
					cursor_pos.x = event.motion.x;
					cursor_pos.y = event.motion.y;
					current->Motion(event.motion.x-old.x, event.motion.y-old.y);
					break;
				}

				case SDL_CONTROLLERAXISMOTION:
					if (has_controller) {
						if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) PadAxis(current, 0, event.caxis.value);
						else if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) PadAxis(current, 1, event.caxis.value);
					}
					break;
				case SDL_CONTROLLERBUTTONDOWN:
				case SDL_CONTROLLERBUTTONUP:
					if (has_controller)
						PadButton(current, event.cbutton.button, event.cbutton.state == SDL_PRESSED);
					break;

				case SDL_JOYAXISMOTION:
					if (!has_controller && Winsys.joystick_isActive())
						PadAxis(current, event.jaxis.axis, event.jaxis.value);
					break;
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
					if (!has_controller && Winsys.joystick_isActive())
						PadButton(current, RawToPadButton(event.jbutton.button), event.jbutton.state == SDL_PRESSED);
					break;
				case SDL_JOYHATMOTION:
					if (!has_controller && Winsys.joystick_isActive()) {
						Uint8 v = event.jhat.value;
						PadButton(current, SDL_CONTROLLER_BUTTON_DPAD_UP, (v & SDL_HAT_UP) != 0);
						PadButton(current, SDL_CONTROLLER_BUTTON_DPAD_DOWN, (v & SDL_HAT_DOWN) != 0);
						PadButton(current, SDL_CONTROLLER_BUTTON_DPAD_LEFT, (v & SDL_HAT_LEFT) != 0);
						PadButton(current, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, (v & SDL_HAT_RIGHT) != 0);
					}
					break;

				case SDL_QUIT:
					quit = true;
					break;
			}
		}
	}
}

void State::Manager::CallLoopFunction() {
	check_gl_error();

	float cur_time = SDL_GetTicks() * 1.e-3;
	g_game.time_step = cur_time - clock_time;
	if (g_game.time_step < 0.0001) g_game.time_step = 0.0001;
	clock_time = cur_time;

	current->Loop(g_game.time_step);
}
