#include "dynos.cpp.h"
extern "C" {
#include "pc/configfile.h"
#include "audio/external.h"
#include "game/game_init.h"
#include "pc/controller/controller_keyboard.h"
#ifdef BETTERCAMERA
#include "game/bettercamera.h"
#endif
}

//
// Data
//

static DynosOption *sPrevOpt     = NULL;
static DynosOption *sDynosMenu   = NULL;
static DynosOption *sOptionsMenu = NULL;
static DynosOption *sCurrentMenu = NULL;
static DynosOption *sCurrentOpt  = NULL;
extern s32 sBindingState;

//
// Action list
//

typedef bool (*DynosActionFunction)(const char *);
struct DynosAction : NoCopy {
    String mFuncName;
    DynosActionFunction mAction;
};

STATIC_STORAGE(Array<DynosAction *>, DynosActions);
#define sDynosActions __DynosActions()

static DynosActionFunction DynOS_Opt_GetAction(const String& aFuncName) {
    for (auto &_DynosAction : sDynosActions) {
        if (_DynosAction->mFuncName == aFuncName) {
            return _DynosAction->mAction;
        }
    }
    return NULL;
}

void DynOS_Opt_AddAction(const String& aFuncName, bool (*aFuncPtr)(const char *), bool aOverwrite) {
    for (auto &_DynosAction : sDynosActions) {
        if (_DynosAction->mFuncName == aFuncName) {
            if (aOverwrite) {
                _DynosAction->mAction = aFuncPtr;
            }
            return;
        }
    }
    DynosAction *_DynosAction = New<DynosAction>();
    _DynosAction->mFuncName = aFuncName;
    _DynosAction->mAction = aFuncPtr;
    sDynosActions.Add(_DynosAction);
}

//
// Constructors
//

static DynosOption *DynOS_Opt_GetExistingOption(DynosOption *aOpt, const String &aName) {
    while (aOpt) {
        if (aOpt->mName == aName) {
            return aOpt;
        }
        if (aOpt->mType == DOPT_SUBMENU) {
            DynosOption *_Opt = DynOS_Opt_GetExistingOption(aOpt->mSubMenu.mChild, aName);
            if (_Opt) {
                return _Opt;
            }
        }
        aOpt = aOpt->mNext;
    }
    return NULL;
}

static DynosOption *DynOS_Opt_NewOption(const String &aName, const String &aConfigName, const String &aLabel, const String &aTitle) {

    // Check if the option already exists
    static DynosOption sDummyOpt;
    if (DynOS_Opt_GetExistingOption(sDynosMenu, aName)) {
        return &sDummyOpt;
    }

    // Create a new option
    DynosOption *_Opt = New<DynosOption>();
    _Opt->mName       = aName;
    _Opt->mConfigName = aConfigName;
    _Opt->mLabel      = { aLabel, NULL };
    _Opt->mTitle      = { aTitle, NULL };
    _Opt->mDynos      = true;
    if (sPrevOpt == NULL) { // The very first option
        _Opt->mPrev   = NULL;
        _Opt->mNext   = NULL;
        _Opt->mParent = NULL;
        sDynosMenu    = _Opt;
    } else {
    if (sPrevOpt->mType == DOPT_SUBMENU && sPrevOpt->mSubMenu.mEmpty) { // First option of a sub-menu
        _Opt->mPrev   = NULL;
        _Opt->mNext   = NULL;
        _Opt->mParent = sPrevOpt;
        sPrevOpt->mSubMenu.mChild = _Opt;
        sPrevOpt->mSubMenu.mEmpty = false;
    } else {
        _Opt->mPrev   = sPrevOpt;
        _Opt->mNext   = NULL;
        _Opt->mParent = sPrevOpt->mParent;
        sPrevOpt->mNext = _Opt;
    }
    }
    sPrevOpt = _Opt;
    return _Opt;
}

static void DynOS_Opt_EndSubMenu() {
    if (sPrevOpt && sPrevOpt->mParent) {
        if (sPrevOpt->mType == DOPT_SUBMENU && sPrevOpt->mSubMenu.mEmpty) { // ENDMENU command following a SUBMENU command
            sPrevOpt->mSubMenu.mEmpty = false;
        } else {
            sPrevOpt = sPrevOpt->mParent;
        }
    }
}

static void DynOS_Opt_CreateSubMenu(const String &aName, const String &aLabel, const String &aTitle) {
    DynosOption *_Opt     = DynOS_Opt_NewOption(aName, "", aLabel, aTitle);
    _Opt->mType           = DOPT_SUBMENU;
    _Opt->mSubMenu.mChild = NULL;
    _Opt->mSubMenu.mEmpty = true;
}

static void DynOS_Opt_CreateToggle(const String &aName, const String &aConfigName, const String &aLabel, s32 aValue) {
    DynosOption *_Opt   = DynOS_Opt_NewOption(aName, aConfigName, aLabel, aLabel);
    _Opt->mType         = DOPT_TOGGLE;
    _Opt->mToggle.mTog  = New<bool>();
    *_Opt->mToggle.mTog = (bool) aValue;
}

static void DynOS_Opt_CreateScroll(const String &aName, const String &aConfigName, const String &aLabel, s32 aMin, s32 aMax, s32 aStep, s32 aValue) {
    DynosOption *_Opt     = DynOS_Opt_NewOption(aName, aConfigName, aLabel, aLabel);
    _Opt->mType           = DOPT_SCROLL;
    _Opt->mScroll.mMin    = aMin;
    _Opt->mScroll.mMax    = aMax;
    _Opt->mScroll.mStep   = aStep;
    _Opt->mScroll.mValue  = New<s32>();
    *_Opt->mScroll.mValue = aValue;
}

static void DynOS_Opt_CreateChoice(const String &aName, const String &aConfigName, const String &aLabel, const Array<String>& aChoices, s32 aValue) {
    DynosOption *_Opt      = DynOS_Opt_NewOption(aName, aConfigName, aLabel, aLabel);
    _Opt->mType            = DOPT_CHOICE;
    _Opt->mChoice.mIndex   = New<s32>();
    *_Opt->mChoice.mIndex  = aValue;
    for (const auto &_Choice : aChoices) {
        _Opt->mChoice.mChoices.Add({ _Choice, NULL });
    }
}

static void DynOS_Opt_CreateButton(const String &aName, const String &aLabel, const String& aFuncName) {
    DynosOption *_Opt       = DynOS_Opt_NewOption(aName, "", aLabel, aLabel);
    _Opt->mType             = DOPT_BUTTON;
    _Opt->mButton.mFuncName = aFuncName;
}

static void DynOS_Opt_CreateBind(const String &aName, const String &aConfigName, const String &aLabel, u32 aMask, u32 aBind0, u32 aBind1, u32 aBind2) {
    DynosOption *_Opt     = DynOS_Opt_NewOption(aName, aConfigName, aLabel, aLabel);
    _Opt->mType           = DOPT_BIND;
    _Opt->mBind.mMask     = aMask;
    _Opt->mBind.mBinds    = New<u32>(3);
    _Opt->mBind.mBinds[0] = aBind0;
    _Opt->mBind.mBinds[1] = aBind1;
    _Opt->mBind.mBinds[2] = aBind2;
    _Opt->mBind.mIndex    = 0;
}

static void DynOS_Opt_ReadFile(const SysPath &aFolder, const SysPath &aFilename) {

    // Open file
    SysPath _FullFilename = fstring("%s/%s", aFolder.c_str(), aFilename.c_str());
    FILE *_File = fopen(_FullFilename.c_str(), "rt");
    if (_File == NULL) {
        return;
    }

    // Read file and create options
    char _Buffer[4096];
    while (fgets(_Buffer, 4096, _File) != NULL) {
        Array<String> _Tokens = Split(_Buffer, " #\t\r\n\b", "#", true);

        // Empty line
        if (_Tokens.Empty()) {
            continue;
        }

        // SUBMENU [Name] [Label] [Title]
        if (_Tokens[0] == "SUBMENU" && _Tokens.Count() >= 4) {
            DynOS_Opt_CreateSubMenu(_Tokens[1], _Tokens[2], _Tokens[3]);
            continue;
        }

        // TOGGLE  [Name] [Label] [ConfigName] [InitialValue]
        if (_Tokens[0] == "TOGGLE" && _Tokens.Count() >= 5) {
            DynOS_Opt_CreateToggle(_Tokens[1], _Tokens[3], _Tokens[2], _Tokens[4].ParseInt());
            continue;
        }

        // SCROLL  [Name] [Label] [ConfigName] [InitialValue] [Min] [Max] [Step]
        if (_Tokens[0] == "SCROLL" && _Tokens.Count() >= 8) {
            DynOS_Opt_CreateScroll(_Tokens[1], _Tokens[3], _Tokens[2], _Tokens[5].ParseInt(), _Tokens[6].ParseInt(), _Tokens[7].ParseInt(), _Tokens[4].ParseInt());
            continue;
        }

        // CHOICE  [Name] [Label] [ConfigName] [InitialValue] [ChoiceStrings...]
        if (_Tokens[0] == "CHOICE" && _Tokens.Count() >= 6) {
            DynOS_Opt_CreateChoice(_Tokens[1], _Tokens[3], _Tokens[2], Array<String>(_Tokens.begin() + 5, _Tokens.end()), _Tokens[4].ParseInt());
            continue;
        }

        // BUTTON  [Name] [Label] [FuncName]
        if (_Tokens[0] == "BUTTON" && _Tokens.Count() >= 4) {
            DynOS_Opt_CreateButton(_Tokens[1], _Tokens[2], _Tokens[3]);
            continue;
        }

        // BIND    [Name] [Label] [ConfigName] [Mask] [DefaultValues]
        if (_Tokens[0] == "BIND" && _Tokens.Count() >= 8) {
            DynOS_Opt_CreateBind(_Tokens[1], _Tokens[3], _Tokens[2], _Tokens[4].ParseInt(), _Tokens[5].ParseInt(), _Tokens[6].ParseInt(), _Tokens[7].ParseInt());
            continue;
        }

        // ENDMENU
        if (_Tokens[0] == "ENDMENU") {
            DynOS_Opt_EndSubMenu();
            continue;
        }
    }
    fclose(_File);
}

static void DynOS_Opt_LoadOptions() {
    SysPath _DynosFolder = fstring("%s/%s", DYNOS_EXE_FOLDER, DYNOS_RES_FOLDER);
    DIR *_DynosDir = opendir(_DynosFolder.c_str());
    sPrevOpt = NULL;
    if (_DynosDir) {
        struct dirent *_DynosEnt = NULL;
        while ((_DynosEnt = readdir(_DynosDir)) != NULL) {
            SysPath _Filename = SysPath(_DynosEnt->d_name);
            if (_Filename.find(".txt") == _Filename.length() - 4) {
                DynOS_Opt_ReadFile(_DynosFolder, _Filename);
            }
        }
        closedir(_DynosDir);
    }
}

//
// Loop through DynosOptions
//

DynosOption *DynOS_Opt_Loop(DynosOption *aOpt, DynosLoopFunc aFunc, void *aData) {
    while (aOpt) {
        if (aFunc(aOpt, aData)) {
            return aOpt;
        } else if (aOpt->mType == DOPT_SUBMENU) {
            DynosOption *_Opt = DynOS_Opt_Loop(aOpt->mSubMenu.mChild, aFunc, aData);
            if (_Opt) {
                return _Opt;
            }
        }
        aOpt = aOpt->mNext;
    }
    return NULL;
}

//
// Get/Set values
//

static bool DynOS_Opt_Get(DynosOption *aOpt, void *aData) {
    return aOpt->mName == (const char *) aData;
}

s32 DynOS_Opt_GetValue(const String &aName) {
    DynosOption *_Opt = DynOS_Opt_Loop(sDynosMenu, DynOS_Opt_Get, (void *) aName.begin());
    if (_Opt) {
        switch (_Opt->mType) {
            case DOPT_TOGGLE:      return *_Opt->mToggle.mTog;
            case DOPT_CHOICE:      return *_Opt->mChoice.mIndex;
            case DOPT_CHOICELEVEL: return *_Opt->mChoice.mIndex;
            case DOPT_CHOICEAREA:  return *_Opt->mChoice.mIndex;
            case DOPT_CHOICESTAR:  return *_Opt->mChoice.mIndex;
#ifndef DYNOS_COOP
            case DOPT_CHOICEPARAM: return *_Opt->mChoice.mIndex;
#endif
            case DOPT_SCROLL:      return *_Opt->mScroll.mValue;
            default:               break;
        }
    }
    return 0;
}

void DynOS_Opt_SetValue(const String &aName, s32 aValue) {
    DynosOption *_Opt = DynOS_Opt_Loop(sDynosMenu, DynOS_Opt_Get, (void *) aName.begin());
    if (_Opt) {
        switch (_Opt->mType) {
            case DOPT_TOGGLE:      *_Opt->mToggle.mTog   = aValue; break;
            case DOPT_CHOICE:      *_Opt->mChoice.mIndex = aValue; break;
            case DOPT_CHOICELEVEL: *_Opt->mChoice.mIndex = aValue; break;
            case DOPT_CHOICEAREA:  *_Opt->mChoice.mIndex = aValue; break;
            case DOPT_CHOICESTAR:  *_Opt->mChoice.mIndex = aValue; break;
#ifndef DYNOS_COOP
            case DOPT_CHOICEPARAM: *_Opt->mChoice.mIndex = aValue; break;
#endif
            case DOPT_SCROLL:      *_Opt->mScroll.mValue = aValue; break;
            default:               break;
        }
    }
}

//
// Processing
//

#define SOUND_DYNOS_SAVED   (SOUND_MENU_MARIO_CASTLE_WARP2  | (0xFF << 8))
#define SOUND_DYNOS_SELECT  (SOUND_MENU_CHANGE_SELECT       | (0xF8 << 8))
#define SOUND_DYNOS_OK      (SOUND_MENU_CHANGE_SELECT       | (0xF8 << 8))
#define SOUND_DYNOS_CANCEL  (SOUND_MENU_CAMERA_BUZZ         | (0xFC << 8))

enum {
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_A,
    INPUT_Z
};

enum {
    RESULT_NONE,
    RESULT_OK,
    RESULT_CANCEL
};

static s32 DynOS_Opt_ProcessInput(DynosOption *aOpt, s32 input) {
    switch (aOpt->mType) {
        case DOPT_TOGGLE:
            if (input == INPUT_LEFT) {
                *aOpt->mToggle.mTog = false;
                return RESULT_OK;
            }
            if (input == INPUT_RIGHT) {
                *aOpt->mToggle.mTog = true;
                return RESULT_OK;
            }
            if (input == INPUT_A) {
                *aOpt->mToggle.mTog = !(*aOpt->mToggle.mTog);
                return RESULT_OK;
            }
            break;

        case DOPT_CHOICE:
            if (input == INPUT_LEFT) {
                *aOpt->mChoice.mIndex = (*aOpt->mChoice.mIndex + aOpt->mChoice.mChoices.Count() - 1) % (aOpt->mChoice.mChoices.Count());
                return RESULT_OK;
            }
            if (input == INPUT_RIGHT || input == INPUT_A) {
                *aOpt->mChoice.mIndex = (*aOpt->mChoice.mIndex + 1) % (aOpt->mChoice.mChoices.Count());
                return RESULT_OK;
            }
            break;

        case DOPT_CHOICELEVEL:
            if (input == INPUT_LEFT) {
                *aOpt->mChoice.mIndex = (*aOpt->mChoice.mIndex + DynOS_Level_GetCount() - 1) % (DynOS_Level_GetCount());
                return RESULT_OK;
            }
            if (input == INPUT_RIGHT || input == INPUT_A) {
                *aOpt->mChoice.mIndex = (*aOpt->mChoice.mIndex + 1) % (DynOS_Level_GetCount());
                return RESULT_OK;
            }
            break;

        case DOPT_CHOICEAREA:
            if (input == INPUT_LEFT) {
                *aOpt->mChoice.mIndex = (*aOpt->mChoice.mIndex + 3) % (4);
                return RESULT_OK;
            }
            if (input == INPUT_RIGHT || input == INPUT_A) {
                *aOpt->mChoice.mIndex = (*aOpt->mChoice.mIndex + 1) % (4);
                return RESULT_OK;
            }
            break;

        case DOPT_CHOICESTAR:
            if (input == INPUT_LEFT) {
                *aOpt->mChoice.mIndex = (*aOpt->mChoice.mIndex + 5) % (6);
                return RESULT_OK;
            }
            if (input == INPUT_RIGHT || input == INPUT_A) {
                *aOpt->mChoice.mIndex = (*aOpt->mChoice.mIndex + 1) % (6);
                return RESULT_OK;
            }
            break;

#ifndef DYNOS_COOP
        case DOPT_CHOICEPARAM:
            if (input == INPUT_LEFT) {
                *aOpt->mChoice.mIndex = (*aOpt->mChoice.mIndex + 4) % (5);
                return RESULT_OK;
            }
            if (input == INPUT_RIGHT || input == INPUT_A) {
                *aOpt->mChoice.mIndex = (*aOpt->mChoice.mIndex + 1) % (5);
                return RESULT_OK;
            }
            break;
#endif

        case DOPT_SCROLL:
            if (input == INPUT_LEFT) {
                *aOpt->mScroll.mValue = MAX(aOpt->mScroll.mMin, *aOpt->mScroll.mValue - aOpt->mScroll.mStep * (gPlayer1Controller->buttonDown & A_BUTTON ? 5 : 1));
                return RESULT_OK;
            }
            if (input == INPUT_RIGHT) {
                *aOpt->mScroll.mValue = MIN(aOpt->mScroll.mMax, *aOpt->mScroll.mValue + aOpt->mScroll.mStep * (gPlayer1Controller->buttonDown & A_BUTTON ? 5 : 1));
                return RESULT_OK;
            }
            break;

        case DOPT_BIND:
            if (input == INPUT_LEFT) {
                aOpt->mBind.mIndex = MAX(0, aOpt->mBind.mIndex - 1);
                return RESULT_OK;
            }
            if (input == INPUT_RIGHT) {
                aOpt->mBind.mIndex = MIN(2, aOpt->mBind.mIndex + 1);
                return RESULT_OK;
            }
            if (input == INPUT_Z) {
                aOpt->mBind.mBinds[aOpt->mBind.mIndex] = VK_INVALID;
                return RESULT_OK;
            }
            if (input == INPUT_A) {
                aOpt->mBind.mBinds[aOpt->mBind.mIndex] = VK_INVALID;
                sBindingState = 1;
                controller_get_raw_key();
                return RESULT_OK;
            }
            break;

        case DOPT_BUTTON:
            if (input == INPUT_A) {
                DynosActionFunction _Action = DynOS_Opt_GetAction(aOpt->mButton.mFuncName);
                if (_Action != NULL && _Action(aOpt->mName.begin())) {
                    return RESULT_OK;
                }
                return RESULT_CANCEL;
            }
            break;

        case DOPT_SUBMENU:
            if (input == INPUT_A) {
                if (aOpt->mSubMenu.mChild != NULL) {
                    sCurrentOpt = aOpt->mSubMenu.mChild;
                    return RESULT_OK;
                }
                return RESULT_CANCEL;
            }
            break;
    }
    return RESULT_NONE;
}

static void DynOS_Opt_Open(DynosOption *aMenu) {
    play_sound(SOUND_DYNOS_SELECT, gDefaultSoundArgs);
    sCurrentMenu = aMenu;
    sCurrentOpt = aMenu;
}

static void DynOS_Opt_Close(bool aPlaySavedSfx) {
    if (sCurrentMenu != NULL) {
        if (aPlaySavedSfx) {
            play_sound(SOUND_DYNOS_SAVED, gDefaultSoundArgs);
        }
#ifdef BETTERCAMERA
        newcam_init_settings();
#endif
        controller_reconfigure();
        configfile_save(configfile_name());
        DynOS_Opt_SaveConfig(sDynosMenu);
        sCurrentMenu = NULL;
    }
}

static void DynOS_Opt_ProcessInputs() {
    static s32 sStickTimer = 0;
    static bool sPrevStick = 0;

    // Stick values
    f32 _StickX = gPlayer1Controller->stickX;
    f32 _StickY = gPlayer1Controller->stickY;
    if (absx(_StickX) > 60 || absx(_StickY) > 60) {
        if (sStickTimer == 0) {
            sStickTimer = (sPrevStick ? 2 : 9);
        } else {
            _StickX = 0;
            _StickY = 0;
            sStickTimer--;
        }
        sPrevStick = true;
    } else {
        sStickTimer = 0;
        sPrevStick = false;
    }

    // Key binding
    if (sBindingState != 0) {
        u32 _Key = (sCurrentOpt->mDynos ? (u32) DynOS_Opt_ControllerGetKeyPressed() : controller_get_raw_key());
        if (_Key != VK_INVALID) {
            play_sound(SOUND_DYNOS_SELECT, gDefaultSoundArgs);
            sCurrentOpt->mBind.mBinds[sCurrentOpt->mBind.mIndex] = _Key;
            sBindingState = false;
        }
        return;
    }

    if (sCurrentMenu != NULL) {

        // Up
        if (_StickY > +60) {
            if (sCurrentOpt->mPrev != NULL) {
                sCurrentOpt = sCurrentOpt->mPrev;
            } else {
                while (sCurrentOpt->mNext) sCurrentOpt = sCurrentOpt->mNext;
            }
            play_sound(SOUND_DYNOS_SELECT, gDefaultSoundArgs);
            return;
        }

        // Down
        if (_StickY < -60) {
            if (sCurrentOpt->mNext != NULL) {
                sCurrentOpt = sCurrentOpt->mNext;
            } else {
                while (sCurrentOpt->mPrev) sCurrentOpt = sCurrentOpt->mPrev;
            }
            play_sound(SOUND_DYNOS_SELECT, gDefaultSoundArgs);
            return;
        }

        // Left
        if (_StickX < -60) {
            switch (DynOS_Opt_ProcessInput(sCurrentOpt, INPUT_LEFT)) {
                case RESULT_OK:     play_sound(SOUND_DYNOS_OK, gDefaultSoundArgs); break;
                case RESULT_CANCEL: play_sound(SOUND_DYNOS_CANCEL, gDefaultSoundArgs); break;
                case RESULT_NONE:   break;
            }
            return;
        }

        // Right
        if (_StickX > +60) {
            switch (DynOS_Opt_ProcessInput(sCurrentOpt, INPUT_RIGHT)) {
                case RESULT_OK:     play_sound(SOUND_DYNOS_OK, gDefaultSoundArgs); break;
                case RESULT_CANCEL: play_sound(SOUND_DYNOS_CANCEL, gDefaultSoundArgs); break;
                case RESULT_NONE:   break;
            }
            return;
        }

        // A
        if (gPlayer1Controller->buttonPressed & A_BUTTON) {
            switch (DynOS_Opt_ProcessInput(sCurrentOpt, INPUT_A)) {
                case RESULT_OK:     play_sound(SOUND_DYNOS_OK, gDefaultSoundArgs); break;
                case RESULT_CANCEL: play_sound(SOUND_DYNOS_CANCEL, gDefaultSoundArgs); break;
                case RESULT_NONE:   break;
            }
            return;
        }

        // B
        if (gPlayer1Controller->buttonPressed & B_BUTTON) {
            if (sCurrentOpt->mParent != NULL) {
                sCurrentOpt = sCurrentOpt->mParent;
                play_sound(SOUND_DYNOS_SELECT, gDefaultSoundArgs);
            } else {
                DynOS_Opt_Close(true);
            }
            return;
        }

        // Z
        if (gPlayer1Controller->buttonPressed & Z_TRIG) {
            switch (DynOS_Opt_ProcessInput(sCurrentOpt, INPUT_Z)) {
                case RESULT_OK:     play_sound(SOUND_DYNOS_OK, gDefaultSoundArgs); break;
                case RESULT_CANCEL: play_sound(SOUND_DYNOS_CANCEL, gDefaultSoundArgs); break;
                case RESULT_NONE:
                    if (sCurrentMenu == sDynosMenu) {
                        DynOS_Opt_Close(true);
                    } else {
                        DynOS_Opt_Open(sDynosMenu);
                    } break;
            }
            return;
        }

        // R
        if (gPlayer1Controller->buttonPressed & R_TRIG) {
            if (sCurrentMenu == sOptionsMenu) {
                DynOS_Opt_Close(true);
            } else {
                DynOS_Opt_Open(sOptionsMenu);
            }
            return;
        }

        // Start
        if (gPlayer1Controller->buttonPressed & START_BUTTON) {
            DynOS_Opt_Close(true);
            return;
        }
    } else if (gPlayer1Controller->buttonPressed & R_TRIG) {
        DynOS_Opt_Open(sOptionsMenu);
    } else if (gPlayer1Controller->buttonPressed & Z_TRIG) {
        DynOS_Opt_Open(sDynosMenu);
    }
}

//
// Init
//

static void DynOS_Opt_CreateWarpToLevelSubMenu() {
    DynOS_Opt_CreateSubMenu("dynos_warp_to_level_submenu", "Warp to Level", "WARP TO LEUEL");

    // Level select
    {
    DynosOption *aOpt     = DynOS_Opt_NewOption("dynos_warp_level", "", "Level Select", "");
    aOpt->mType           = DOPT_CHOICELEVEL;
    aOpt->mChoice.mIndex  = New<s32>();
    *aOpt->mChoice.mIndex = 0;
    }

    // Area select
    {
    DynosOption *aOpt     = DynOS_Opt_NewOption("dynos_warp_area", "", "Area Select", "");
    aOpt->mType           = DOPT_CHOICEAREA;
    aOpt->mChoice.mIndex  = New<s32>();
    *aOpt->mChoice.mIndex = 0;
    }

    // Star select
    {
    DynosOption *aOpt     = DynOS_Opt_NewOption("dynos_warp_act", "", "Star Select", "");
    aOpt->mType           = DOPT_CHOICESTAR;
    aOpt->mChoice.mIndex  = New<s32>();
    *aOpt->mChoice.mIndex = 0;
    }

#ifndef DYNOS_COOP
    // Param select
    {
    DynosOption *aOpt = DynOS_Opt_NewOption("dynos_warp_param", "", "Param Select", "");
    aOpt->mType           = DOPT_CHOICEPARAM;
    aOpt->mChoice.mIndex  = New<s32>();
    *aOpt->mChoice.mIndex = 0;
    }
#endif

    DynOS_Opt_CreateButton("dynos_warp_to_level", "Warp", "DynOS_Opt_WarpToLevel");
    DynOS_Opt_EndSubMenu();
}

static void DynOS_Opt_CreateWarpToCastleSubMenu() {
    DynOS_Opt_CreateSubMenu("dynos_warp_to_castle_submenu", "Warp to Castle", "WARP TO CASTLE");

    // Level select
    {
    DynosOption *aOpt     = DynOS_Opt_NewOption("dynos_warp_castle", "", "Level Exit", "");
    aOpt->mType           = DOPT_CHOICELEVEL;
    aOpt->mChoice.mIndex  = New<s32>();
    *aOpt->mChoice.mIndex = 0;
    }

    DynOS_Opt_CreateButton("dynos_warp_to_castle", "Warp", "DynOS_Opt_WarpToCastle");
    DynOS_Opt_EndSubMenu();
}

static u32 DynOS_Opt_GetHash(const String& aStr) {
    u32 _Hash = 5381u;
    for (char c : aStr) { _Hash += c + (_Hash << 5); }
    return _Hash;
}

static void DynOS_Opt_CreateModelPacksSubMenu() {
    Array<String> _Packs = DynOS_Gfx_Init();
    if (_Packs.Count() == 0) {
        return;
    }

    DynOS_Opt_CreateSubMenu("dynos_model_loader_submenu", "Model Packs", "MODEL PACKS");
    for (s32 i = 0; i != _Packs.Count(); ++i) {
        DynOS_Opt_CreateToggle(String("dynos_pack_%d", i), String("dynos_pack_%08X", DynOS_Opt_GetHash(_Packs[i])), _Packs[i], false);
    }
    DynOS_Opt_CreateButton("dynos_packs_disable_all", "Disable all packs", "DynOS_Opt_DisableAllPacks");
    DynOS_Opt_EndSubMenu();
}

void DynOS_Opt_Init() {

    // Convert options menu
    DynOS_Opt_InitVanilla(sOptionsMenu);

    // Create DynOS menu
    DynOS_Opt_LoadOptions();

    // Warp to level
    DynOS_Opt_CreateWarpToLevelSubMenu();

    // Warp to castle
    DynOS_Opt_CreateWarpToCastleSubMenu();

    // Restart level
    DynOS_Opt_CreateButton("dynos_restart_level", "Restart Level", "DynOS_Opt_RestartLevel");

    // Exit level
    DynOS_Opt_CreateButton("dynos_exit_level", "Exit Level", "DynOS_Opt_ExitLevel");

#ifndef DYNOS_COOP
    // Return to main menu
    DynOS_Opt_CreateButton("dynos_return_to_main_menu", "Return to Main Menu", "DynOS_Opt_ReturnToMainMenu");
#endif

    // Model loader
    DynOS_Opt_CreateModelPacksSubMenu();

    // Init config
    DynOS_Opt_LoadConfig(sDynosMenu);
}

//
// Update
//

void DynOS_Opt_Update(OSContPad *aPad) {
    DynOS_Opt_Loop(sDynosMenu, DynOS_Opt_ControllerUpdate, (void *) aPad);
    if (DynOS_IsTransitionActive()) {
        aPad->button = 0;
        aPad->stick_x = 0;
        aPad->stick_y = 0;
        aPad->ext_stick_x = 0;
        aPad->ext_stick_y = 0;
    }
}

//
// Hijack
// This is C code
//

extern "C" {

u8 optmenu_open = 0;

void optmenu_toggle(void) {
    DynOS_Opt_Close(false);
    optmenu_open = 0;
}

void optmenu_draw(void) {
    DynOS_Opt_DrawMenu(sCurrentOpt, sCurrentMenu, sOptionsMenu, sDynosMenu);
}

void optmenu_draw_prompt(void) {
    DynOS_Opt_DrawPrompt(sCurrentMenu, sOptionsMenu, sDynosMenu);
    DynOS_Opt_ProcessInputs();
    optmenu_open = (sCurrentMenu != NULL);
}

void optmenu_check_buttons(void) {
}

}

//
// Built-in options
//

#define DYNOS_DEFINE_ACTION(func) \
DYNOS_AT_STARTUP static void DynOS_Opt_AddAction_##func() { \
    DynOS_Opt_AddAction(#func, func, false); \
}

#ifndef DYNOS_COOP
static bool DynOS_Opt_ReturnToMainMenu(UNUSED const char *optName) {
    DynOS_ReturnToMainMenu();
    return true;
}
DYNOS_DEFINE_ACTION(DynOS_Opt_ReturnToMainMenu);
#endif

static bool DynOS_Opt_WarpToLevel(UNUSED const char *optName) {
    s32 _Level = DynOS_Level_GetList()[DynOS_Opt_GetValue("dynos_warp_level")];
    s32 _Area = DynOS_Opt_GetValue("dynos_warp_area") + 1;
    s32 _Act = DynOS_Opt_GetValue("dynos_warp_act") + 1;
#ifdef DYNOS_COOP
    DynOS_Coop_SendCommand(DYNOS_COOP_COMMAND_WARP_TO_LEVEL, _Level, _Area, _Act);
#endif
    return DynOS_Warp_ToLevel(_Level, _Area, _Act);
}
DYNOS_DEFINE_ACTION(DynOS_Opt_WarpToLevel);

static bool DynOS_Opt_WarpToCastle(UNUSED const char *optName) {
    s32 _Level = DynOS_Level_GetList()[DynOS_Opt_GetValue("dynos_warp_castle")];
#ifdef DYNOS_COOP
    DynOS_Coop_SendCommand(DYNOS_COOP_COMMAND_WARP_TO_CASTLE, _Level, 0, 0);
#endif
    return DynOS_Warp_ToCastle(_Level);
}
DYNOS_DEFINE_ACTION(DynOS_Opt_WarpToCastle);

static bool DynOS_Opt_RestartLevel(UNUSED const char *optName) {
#ifdef DYNOS_COOP
    DynOS_Coop_SendCommand(DYNOS_COOP_COMMAND_RESTART_LEVEL, 0, 0, 0);
#endif
    return DynOS_Warp_RestartLevel();
}
DYNOS_DEFINE_ACTION(DynOS_Opt_RestartLevel);

static bool DynOS_Opt_ExitLevel(UNUSED const char *optName) {
#ifdef DYNOS_COOP
    DynOS_Coop_SendCommand(DYNOS_COOP_COMMAND_EXIT_LEVEL, 0, 0, 0);
#endif
    return DynOS_Warp_ExitLevel(30);
}
DYNOS_DEFINE_ACTION(DynOS_Opt_ExitLevel);

static bool DynOS_Opt_DisableAllPacks(UNUSED const char *optName) {
    const Array<PackData *> &pDynosPacks = DynOS_Gfx_GetPacks();
    for (s32 i = 0; i != pDynosPacks.Count(); ++i) {
        DynOS_Opt_SetValue(String("dynos_pack_%d", i), false);
    }
    return true;
}
DYNOS_DEFINE_ACTION(DynOS_Opt_DisableAllPacks);

#undef DYNOS_DEFINE_ACTION
