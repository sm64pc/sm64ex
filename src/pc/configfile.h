#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <stdbool.h>

#define CONFIGFILE_DEFAULT "moon64config.txt"

#define MAX_BINDS  3
#define MAX_VOLUME 127

typedef struct {
    unsigned int x, y, w, h;
    bool vsync;
    bool reset;
    bool fullscreen;
    bool exiting_fullscreen;
    bool settings_changed;
    bool enable_antialias;
    unsigned int antialias_level;
    float internal_w, internal_h;
    float multiplier;
} ConfigWindow;

typedef struct {
    bool s_stats;
    bool s_toggles;
    bool s_machinima;
    bool s_appearance;
    bool s_options;
    bool texture_debug;
    bool jaboMode;
    bool wireframeMode;
} ImGuiConfig;

extern ConfigWindow configWindow;
extern ImGuiConfig  configImGui;
extern unsigned int configLanguage;
#ifdef TARGET_SWITCH
extern bool         configSwitchHud;
#endif
extern unsigned int configColorCode;
extern unsigned int configFiltering;
extern unsigned int configMasterVolume;
extern unsigned int configMusicVolume;
extern unsigned int configSfxVolume;
extern unsigned int configEnvVolume;
extern bool         configVoicesEnabled;
extern unsigned int configKeyA[];
extern unsigned int configKeyB[];
extern unsigned int configKeyStart[];
extern unsigned int configKeyL[];
extern unsigned int configKeyR[];
extern unsigned int configKeyZ[];
extern unsigned int configKeyCUp[];
extern unsigned int configKeyCDown[];
extern unsigned int configKeyCLeft[];
extern unsigned int configKeyCRight[];
extern unsigned int configKeyStickUp[];
extern unsigned int configKeyStickDown[];
extern unsigned int configKeyStickLeft[];
extern unsigned int configKeyStickRight[];
extern unsigned int configStickDeadzone;
extern unsigned int configRumbleStrength;
extern bool         configPrecacheRes;
#ifdef BETTERCAMERA
extern unsigned int configCameraXSens;
extern unsigned int configCameraYSens;
extern unsigned int configCameraAggr;
extern unsigned int configCameraPan;
extern unsigned int configCameraDegrade;
extern bool         configCameraInvertX;
extern bool         configCameraInvertY;
extern bool         configEnableCamera;
extern bool         configCameraMouse;
extern bool         configCameraAnalog;
#endif
extern bool         configHUD;
extern bool         configSkipIntro;
#ifdef DISCORDRPC
extern bool         configDiscordRPC;
#endif

extern unsigned int configLODMode;

void configfile_load(const char *filename);
void configfile_save(const char *filename);
const char *configfile_name(void);

#endif
