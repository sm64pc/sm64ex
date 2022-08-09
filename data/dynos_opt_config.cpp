#include "dynos.cpp.h"

extern DynosOption *DynOS_Opt_Loop(DynosOption *aOpt, DynosLoopFunc aFunc, void *aData);

static bool DynOS_Opt_ReadConfig(DynosOption *aOpt, void *aData) {
    return (aOpt->mConfigName == (const char *) aData);
}

void DynOS_Opt_LoadConfig(DynosOption *aMenu) {
    SysPath _Filename = fstring("%s/%s", DYNOS_USER_FOLDER, DYNOS_CONFIG_FILENAME);
    FILE *_File = fopen(_Filename.c_str(), "r");
    if (_File) {
        char _Buffer[1024];
        while (fgets(_Buffer, 1024, _File)) {
            
            // Option strings
            char *_NameBegin = _Buffer;
            char *_DataBegin = strchr(_NameBegin, '=');
            if (_NameBegin && _DataBegin) {
                *(_DataBegin++) = 0;

                // Option name
                String _OptName = String(_NameBegin);
                DynosOption *_Opt = DynOS_Opt_Loop(aMenu, DynOS_Opt_ReadConfig, (void *) _OptName.begin());
                if (_Opt) {

                    // Option values
                    switch (_Opt->mType) {
                        case DOPT_TOGGLE: sscanf(_DataBegin, "%hhu\n",           &_Opt->mToggle.mTog[0]); break;
                        case DOPT_CHOICE: sscanf(_DataBegin, "%d\n",             &_Opt->mChoice.mIndex[0]); break;
                        case DOPT_SCROLL: sscanf(_DataBegin, "%d\n",             &_Opt->mScroll.mValue[0]); break;
                        case DOPT_BIND:   sscanf(_DataBegin, "%04X;%04X;%04X\n", &_Opt->mBind.mBinds[0], &_Opt->mBind.mBinds[1], &_Opt->mBind.mBinds[2]); break;
                    }
                }
            }
        }
        fclose(_File);
    }
}

static bool DynOS_Opt_WriteConfig(DynosOption *aOpt, void *aData) {
    if (aOpt->mConfigName.Length() != 0 &&
        aOpt->mConfigName          != "null" &&
        aOpt->mConfigName          != "NULL") {
        switch (aOpt->mType) {
            case DOPT_TOGGLE: fprintf((FILE *) aData, "%s=%hhu\n",           aOpt->mConfigName.begin(), aOpt->mToggle.mTog[0]); break;
            case DOPT_CHOICE: fprintf((FILE *) aData, "%s=%d\n",             aOpt->mConfigName.begin(), aOpt->mChoice.mIndex[0]); break;
            case DOPT_SCROLL: fprintf((FILE *) aData, "%s=%d\n",             aOpt->mConfigName.begin(), aOpt->mScroll.mValue[0]); break;
            case DOPT_BIND:   fprintf((FILE *) aData, "%s=%04X;%04X;%04X\n", aOpt->mConfigName.begin(), aOpt->mBind.mBinds[0], aOpt->mBind.mBinds[1], aOpt->mBind.mBinds[2]); break;
        }
    }
    return false;
}

void DynOS_Opt_SaveConfig(DynosOption *aMenu) {
    SysPath _Filename = fstring("%s/%s", DYNOS_USER_FOLDER, DYNOS_CONFIG_FILENAME);
    FILE *_File = fopen(_Filename.c_str(), "w");
    if (_File) {
        DynOS_Opt_Loop(aMenu, DynOS_Opt_WriteConfig, (void *) _File);
        fclose(_File);
    }
}
