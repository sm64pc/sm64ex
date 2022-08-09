#include "dynos.cpp.h"

static DynosOption *sPrevOpt = NULL;
static DynosOption *sOptionsMenu = NULL;

//
// Vanilla actions
//

typedef void (*VanillaActionFunction)(struct Option *, s32);
typedef struct VanillaAction {
    String mFuncName;
    VanillaActionFunction mAction;
} VanillaAction;

STATIC_STORAGE(Array<VanillaAction *>, VanillaActions);
#define sVanillaActions __VanillaActions()

static VanillaActionFunction DynOS_Opt_GetVanillaAction(const String& aFuncName) {
    for (auto &_DynosAction : sVanillaActions) {
        if (_DynosAction->mFuncName == aFuncName) {
            return _DynosAction->mAction;
        }
    }
    return NULL;
}

static void DynOS_Opt_AddVanillaAction(const String& aFuncName, void (*aFuncPtr)(struct Option *, s32)) {
    for (auto &_DynosAction : sVanillaActions) {
        if (_DynosAction->mFuncName == aFuncName) {
            return;
        }
    }
    VanillaAction *_DynosAction = New<VanillaAction>();
    _DynosAction->mFuncName = aFuncName;
    _DynosAction->mAction = aFuncPtr;
    sVanillaActions.Add(_DynosAction);
}

static bool DynOS_Opt_CallVanillaAction(const char *aOptName) {
    VanillaActionFunction _Func = DynOS_Opt_GetVanillaAction(aOptName);
    if (_Func) {
        _Func(NULL, 0);
        return true;
    }
    return false;
}

//
// Convert classic options menu into DynOS menu
//

static DynosOption *DynOS_Opt_ConvertOption(const u8 *aLabel, const u8 *aTitle) {
    static u32 sOptIdx = 0;
    DynosOption *_Opt   = New<DynosOption>();
    _Opt->mName         = String("vanilla_opt_%08X", sOptIdx++);
    _Opt->mConfigName   = "";
    _Opt->mLabel        = { "", aLabel };
    _Opt->mTitle        = { "", aTitle };
    _Opt->mDynos        = false;
    if (sPrevOpt == NULL) { // The very first option
        _Opt->mPrev     = NULL;
        _Opt->mNext     = NULL;
        _Opt->mParent   = NULL;
        sOptionsMenu    = _Opt;
    } else {
    if (sPrevOpt->mType == DOPT_SUBMENU && sPrevOpt->mSubMenu.mEmpty) { // First option of a sub-menu
        _Opt->mPrev     = NULL;
        _Opt->mNext     = NULL;
        _Opt->mParent   = sPrevOpt;
        sPrevOpt->mSubMenu.mChild = _Opt;
        sPrevOpt->mSubMenu.mEmpty = false;
    } else {
        _Opt->mPrev     = sPrevOpt;
        _Opt->mNext     = NULL;
        _Opt->mParent   = sPrevOpt->mParent;
        sPrevOpt->mNext = _Opt;
    }
    }
    sPrevOpt = _Opt;
    return _Opt;
}

static void DynOS_Opt_EndSubMenu() {
    if (sPrevOpt) {
        if (sPrevOpt->mType == DOPT_SUBMENU && sPrevOpt->mSubMenu.mEmpty) { // ENDMENU command following a SUBMENU command
            sPrevOpt->mSubMenu.mEmpty = false;
        } else {
            sPrevOpt = sPrevOpt->mParent;
        }
    }
}

static void DynOS_Opt_ConvertSubMenu(const u8 *aLabel, const u8 *aTitle) {
    DynosOption *_Opt     = DynOS_Opt_ConvertOption(aLabel, aTitle);
    _Opt->mType           = DOPT_SUBMENU;
    _Opt->mSubMenu.mChild = NULL;
    _Opt->mSubMenu.mEmpty = true;
}

static void DynOS_Opt_ConvertToggle(const u8 *aLabel, bool *pValue) {
    DynosOption *_Opt  = DynOS_Opt_ConvertOption(aLabel, aLabel);
    _Opt->mType        = DOPT_TOGGLE;
    _Opt->mToggle.mTog = (bool *) pValue;
}

static void DynOS_Opt_ConvertScroll(const u8 *aLabel, s32 aMin, s32 aMax, s32 aStep, u32 *pValue) {
    DynosOption *_Opt    = DynOS_Opt_ConvertOption(aLabel, aLabel);
    _Opt->mType          = DOPT_SCROLL;
    _Opt->mScroll.mMin   = aMin;
    _Opt->mScroll.mMax   = aMax;
    _Opt->mScroll.mStep  = aStep;
    _Opt->mScroll.mValue = (s32 *) pValue;
}

static void DynOS_Opt_ConvertChoice(const u8 *aLabel, const u8 **aChoices, s32 aCount, u32 *pValue) {
    DynosOption *_Opt    = DynOS_Opt_ConvertOption(aLabel, aLabel);
    _Opt->mType          = DOPT_CHOICE;
    _Opt->mChoice.mIndex = (s32 *) pValue;
    for (s32 i = 0; i != aCount; ++i) {
    _Opt->mChoice.mChoices.Add({ "", aChoices[i] });
    }
}

static void DynOS_Opt_ConvertButton(const u8 *aLabel, VanillaActionFunction aAction) {
    DynosOption *_Opt       = DynOS_Opt_ConvertOption(aLabel, aLabel);
    _Opt->mType             = DOPT_BUTTON;
    _Opt->mButton.mFuncName = "DynOS_Opt_CallVanillaAction";
    DynOS_Opt_AddVanillaAction(_Opt->mName, aAction);
}

static void DynOS_Opt_ConvertBind(const u8 *aLabel, u32 *pBinds) {
    DynosOption *_Opt  = DynOS_Opt_ConvertOption(aLabel, aLabel);
    _Opt->mType        = DOPT_BIND;
    _Opt->mBind.mMask  = 0;
    _Opt->mBind.mBinds = pBinds;
    _Opt->mBind.mIndex = 0;
}

extern "C" {
extern void dynos_opt_convert_vanilla_main_menu();
void dynos_opt_end_submenu() { return DynOS_Opt_EndSubMenu(); }
void dynos_opt_convert_submenu(const u8 *label, const u8 *title) { return DynOS_Opt_ConvertSubMenu(label, title); }
void dynos_opt_convert_toggle(const u8 *label, bool *bval) { return DynOS_Opt_ConvertToggle(label, bval); }
void dynos_opt_convert_scroll(const u8 *label, s32 min, s32 max, s32 step, u32 *uval) { return DynOS_Opt_ConvertScroll(label, min, max, step, uval); }
void dynos_opt_convert_choice(const u8 *label, const u8 **choices, s32 numChoices, u32 *uval) { return DynOS_Opt_ConvertChoice(label, choices, numChoices, uval); }
void dynos_opt_convert_button(const u8 *label, void *action) { return DynOS_Opt_ConvertButton(label, (VanillaActionFunction) action); }
void dynos_opt_convert_bind(const u8 *label, u32 *uval) { return DynOS_Opt_ConvertBind(label, uval); }
}
void DynOS_Opt_InitVanilla(DynosOption *&aOptionsMenu) {
    sPrevOpt = NULL;
    dynos_opt_convert_vanilla_main_menu();
    DynOS_Opt_AddAction("DynOS_Opt_CallVanillaAction", DynOS_Opt_CallVanillaAction, true);
    aOptionsMenu = sOptionsMenu;
}
