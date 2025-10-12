#include <string>
// --- Extreme Tux Racer (Miyoo/Onion port) header cleanup ---
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

// Where the packaged assets will live on device (adjust if you prefer)

#include "game_config.h"
#include "spx.h"
#include "translation.h"
// --- end header ---
static std::string GetAppDir(){
  char buf[1024];
  ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf)-1);
  if(n > 0){
    buf[n] = 0;
    std::string p(buf);
    size_t slash = p.find_last_of('/');
    if(slash != std::string::npos) p.resize(slash);
    return p;
  }
  return std::string(".");
}
TParam param;


void LoadConfigFile() {
	CSPList list(4);
	if (!list.Load(param.configfile)) {
		Message("Could not load 'options'");
		return;
	}

	for (CSPList::const_iterator line = list.cbegin(); line != list.cend(); ++line) {
		param.fullscreen = SPBoolN(*line, "fullscreen", false);
		param.res_type = SPIntN(*line, "res_type", 0);
		param.framerate = SPIntN(*line, "framerate", 60);
		param.perf_level = SPIntN(*line, "detail_level", 3);
		param.language = Trans.GetLangIdx(SPStrN(*line, "language", "EN_en"));
		param.sound_volume = SPIntN(*line, "sound_volume", 100);
		param.music_volume = SPIntN(*line, "music_volume", 20);

		param.forward_clip_distance = SPIntN(*line, "forward_clip_distance", 75);
		param.backward_clip_distance = SPIntN(*line, "backward_clip_distance", 20);
		param.fov = SPIntN(*line, "fov", 60);
		param.bpp_mode = SPIntN(*line, "bpp_mode", 1);
		param.tree_detail_distance = SPIntN(*line, "tree_detail_distance", 20);
		param.tux_sphere_divisions = SPIntN(*line, "tux_sphere_divisions", 10);
		param.tux_shadow_sphere_divisions = SPIntN(*line, "tux_shadow_sphere_div", 3);
		param.course_detail_level = SPIntN(*line, "course_detail_level", 75);

		param.use_papercut_font = SPIntN(*line, "use_papercut_font", 1);
		param.ice_cursor = SPBoolN(*line, "ice_cursor", true);
		param.full_skybox = SPBoolN(*line, "full_skybox", false);
		param.audio_freq = SPIntN(*line, "audio_freq", 22050);
		param.audio_buffer_size = SPIntN(*line, "audio_buffer_size", 512);
		param.use_quad_scale = SPBoolN(*line, "use_quad_scale", false);

		param.menu_music = SPStrN(*line, "menu_music", "start_1");
		param.credits_music = SPStrN(*line, "credits_music", "credits_1");
		param.config_music = SPStrN(*line, "config_music", "options_1");
	}
}

void SetConfigDefaults() {
	param.fullscreen = true;
	param.res_type = 0; // 0=auto / 1=640x480 / 2=800x600 ...
	param.framerate = 60;
	param.perf_level = 3;	// detail level
	param.language = string::npos; // If language is set to npos, ETR will try to load default system language
	param.sound_volume = 100;
	param.music_volume = 20;

	// ---------------------------------------

	param.forward_clip_distance = 75;
	param.backward_clip_distance = 20;
	param.fov = 60;
	param.bpp_mode = 1;
	param.tree_detail_distance = 20;
	param.tux_sphere_divisions = 10;
	param.tux_shadow_sphere_divisions = 3;
	param.course_detail_level = 75;
	param.audio_freq = 22050;
	param.audio_buffer_size = 512;

	param.use_papercut_font = 1;
	param.ice_cursor = true;
	param.full_skybox = false;
	param.use_quad_scale = false;

	param.menu_music = "start_1";
	param.credits_music = "credits_1";
	param.config_music = "options_1";
}


void AddItem(CSPList &list, const string& tag, const string& content) {
	string item = "  [" +tag + "] " + content;
	list.Add(item);
}

void AddIntItem(CSPList &list, const string& tag, int val) {
	string vs = Int_StrN(val);
	AddItem(list, tag, vs);
}

void AddComment(CSPList &list, const string& comment) {
	string line = "# " + comment;
	list.Add(line);
}

void SaveConfigFile() {
	CSPList liste(512);

	liste.Add("# ------------------------------------------------------------------");
	liste.Add("#   The first group of params can be adjusted ");
	liste.Add("#   on the configuration screen, too");
	liste.Add("# ------------------------------------------------------------------");
	liste.Add();

	AddComment(liste, "Full-screen mode [0...1]");
	AddIntItem(liste, "fullscreen", param.fullscreen);
	liste.Add();

	AddComment(liste, "Screen resolution [0...10]");
	AddComment(liste, "0 = auto, 1 = 640x480, 2 = 800x600, 3 = 1024x768");
	AddComment(liste, "4 = 1152x864, 5 = 1280x960, 6 = 1280x1024");
	AddComment(liste, "7 = 1360x768, 8 = 1400x1050, 9 = 1440x900, 10=1680x1050");
	AddIntItem(liste, "res_type", (int)param.res_type);
	liste.Add();

	AddComment(liste, "Framerate limit");
	AddComment(liste, "0 = unlimited, default: 60");
	AddIntItem(liste, "framerate", (int)param.framerate);
	liste.Add();

	AddComment(liste, "Level of details [1...4]");
	AddComment(liste, "1 = best performance, 4 = best appearance");
	AddIntItem(liste, "detail_level", param.perf_level);
	liste.Add();

	AddComment(liste, "Language code");
	AddComment(liste, "en_GB = English etc.");
	AddItem(liste, "language", Trans.languages[param.language].lang);
	liste.Add();

	AddComment(liste, "Sound volume [0...120]");
	AddComment(liste, "Sounds are the terrain effects or the pickup noise.");
	AddIntItem(liste, "sound_volume", param.sound_volume);
	liste.Add();

	AddComment(liste, "Volume of the background music [0...120]");
	AddIntItem(liste, "music_volume", param.music_volume);
	liste.Add();

	liste.Add("# ------------------------------------------------------------------");
	liste.Add("#   The second group of params must be adjusted in this file.");
	liste.Add("# ------------------------------------------------------------------");
	liste.Add();

	AddComment(liste, "Forward clipping distance");
	AddComment(liste, "Controls how far ahead of the camera the course");
	AddComment(liste, "is rendered. Larger values mean that more of the course is");
	AddComment(liste, "rendered, resulting in slower performance. Decreasing this ");
	AddComment(liste, "value is an effective way to improve framerates.");
	AddIntItem(liste, "forward_clip_distance", param.forward_clip_distance);
	liste.Add();

	AddComment(liste, "Backward clipping distance");
	AddComment(liste, "Some objects aren't yet clipped to the view frustum, ");
	AddComment(liste, "so this value is used to control how far up the course these ");
	AddComment(liste, "objects are drawn.");
	AddIntItem(liste, "backward_clip_distance", param.backward_clip_distance);
	liste.Add();

	AddComment(liste, "Field of View of the camera");
	AddIntItem(liste, "fov", param.fov);
	liste.Add();

	AddComment(liste, "Bpp mode - bits per pixel [0...2]");
	AddComment(liste, "Controls the color depth of the OpenGL window");
	AddComment(liste, "0 = use current bpp setting of operating system,");
	AddComment(liste, "1 = 16 bpp, 2 = 32 bpp");
	AddIntItem(liste, "bpp_mode", param.bpp_mode);
	liste.Add();

	AddComment(liste, "Tree detail distance");
	AddComment(liste, "Controls how far up the course the trees are drawn crosswise.");
	AddIntItem(liste, "tree_detail_distance", param.tree_detail_distance);
	liste.Add();

	AddComment(liste, "Tux sphere divisions");
	AddComment(liste, "Controls how detailled the character is drawn");
	AddIntItem(liste, "tux_sphere_divisions", param.tux_sphere_divisions);
	liste.Add();

	AddComment(liste, "Tux shadow sphere divisions");
	AddComment(liste, "The same but for the shadow of the character");
	AddIntItem(liste, "tux_shadow_sphere_div", param.tux_shadow_sphere_divisions);
	liste.Add();

	AddComment(liste, "Detail level of the course");
	AddComment(liste, "This param is used for the quadtree and controls the");
	AddComment(liste, "LOD of the algorithm. ");
	AddIntItem(liste, "course_detail_level", param.course_detail_level);
	liste.Add();

	AddComment(liste, "Font type [0...2]");
	AddComment(liste, "0 = always arial-like font,");
	AddComment(liste, "1 = papercut font on the menu screens");
	AddComment(liste, "2 = papercut font for the hud display, too");
	AddIntItem(liste, "use_papercut_font", param.use_papercut_font);
	liste.Add();

	AddComment(liste, "Cursor type [0...1]");
	AddComment(liste, "0 = normal cursor (arrow), 1 = icicle");
	AddIntItem(liste, "ice_cursor", param.ice_cursor);
	liste.Add();

	AddComment(liste, "Draw full skybox [0...1]");
	AddComment(liste, "A normal skybox consists of 6 textures. In Tuxracer");
	AddComment(liste, "3 textures are invisible (top, bottom and back).");
	AddComment(liste, "These textures needn't be drawn.");
	AddIntItem(liste, "full_skybox", param.full_skybox);
	liste.Add();

	AddComment(liste, "Audio frequency");
	AddComment(liste, "Typical values are 11025, 22050 ...");
	AddIntItem(liste, "audio_freq", param.audio_freq);
	liste.Add();

	AddComment(liste, "Size of audio buffer");
	AddComment(liste, "Typical values are 512, 1024, 2048 ...");
	AddIntItem(liste, "audio_buffer_size", param.audio_buffer_size);
	liste.Add();

	AddComment(liste, "Select the music:");
	AddComment(liste, "(the racing music is defined by a music theme)");
	AddItem(liste, "menu_music", param.menu_music);
	AddItem(liste, "credits_music", param.credits_music);
	AddItem(liste, "config_music", param.config_music);
	liste.Add();

	AddComment(liste, "Use sqare root of scale factors for menu screens [0...1]");
	AddComment(liste, "Exprimental: these factors reduce the effect of screen scaling.");
	AddComment(liste, "The widgets are closer to their default sizes.");
	AddIntItem(liste, "use_quad_scale", param.use_quad_scale);
	liste.Add();

	// ---------------------------------------
	liste.Save(param.configfile);
}

// --------------------------------------------------------------------

void InitConfig() {
#if defined (OS_WIN32_MINGW) || defined (OS_WIN32_MSC)
	// the progdir is always the current dir
	param.config_dir = "config";
	param.data_dir = "data";
	param.configfile = param.config_dir + SEP "options.txt";
#else /* WIN32 */

#if 0
	char buff[256];

	if (strcmp(arg0, "./etr") == 0) {		// start from work directory
		char *s = getcwd(buff, 256);
		if (s==NULL) {};
	} else {								// start with full path
		strcpy(buff, arg0);
		if (strlen(buff) > 5) {
			buff[strlen(buff)-3] = 0;
		}
	}

	param.prog_dir = buff;
#endif /* 0 */

	struct passwd *pwent = getpwuid(getuid());
	param.config_dir = pwent->pw_dir;
	param.config_dir += SEP;
	param.config_dir += ".etr";
	// or: param.config_dir = param.prog_dir + SEP "config";
	if (!DirExists(param.config_dir.c_str()))
		::mkdir(param.config_dir.c_str(), 0775);
	param.data_dir = GetAppDir() + SEP "data";
	param.data_dir += SEP;
	param.data_dir += "etr";
	// param.data_dir = param.prog_dir + SEP "data";
	param.configfile = param.config_dir + SEP "options";
#endif /* WIN32 */

	param.screenshot_dir = param.data_dir + SEP "screenshots";
	param.obj_dir = param.data_dir + SEP "objects";
	param.env_dir2 = param.data_dir + SEP "env";
	param.char_dir = param.data_dir + SEP "char";
	param.terr_dir = param.data_dir + SEP "terrains";
	param.tex_dir = param.data_dir + SEP "textures";
	param.common_course_dir = param.data_dir + SEP "courses";
	param.sounds_dir = param.data_dir + SEP "sounds";
	param.music_dir = param.data_dir + SEP "music";
	param.font_dir = param.data_dir + SEP "fonts";
	param.trans_dir = param.data_dir + SEP "translations";
	param.player_dir = param.data_dir + SEP "players";

	param.ui_snow = true;
	param.view_mode = FOLLOW;
	param.display_fps = false;
	param.show_hud = true;

	if (FileExists(param.configfile)) {
		Trans.LoadLanguages();
		LoadConfigFile();
	} else {
		SetConfigDefaults();
		Trans.LoadLanguages();
	}
}
