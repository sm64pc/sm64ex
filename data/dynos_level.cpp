#include "dynos.cpp.h"
extern "C" {
#include "game/segment2.h"
#include "game/save_file.h"
#include "levels/scripts.h"
}

//
// Const
//

extern "C" {
extern const BehaviorScript *sWarpBhvSpawnTable[];
}

#define DYNOS_LEVEL_TEXT_EMPTY              ""
#define DYNOS_LEVEL_TEXT_CASTLE             "CASTLE"
#define DYNOS_LEVEL_TEXT_BOWSER_1           "BOWSER 1"
#define DYNOS_LEVEL_TEXT_BOWSER_2           "BOWSER 2"
#define DYNOS_LEVEL_TEXT_BOWSER_3           "BOWSER 3"
#define DYNOS_LEVEL_TEXT_100_COINS_STAR     "100 COINS STAR"
#define DYNOS_LEVEL_TEXT_RED_COINS_STAR     "RED COINS STAR"
#define DYNOS_LEVEL_TEXT_ONE_SECRET_STAR    "ONE OF THE CASTLE'S SECRET STARS!"

static void SetConvertedTextToBuffer(u8 *aBuffer, const char *aText) {
    u8 *_ConvertedText = DynOS_String_Convert(aText, false);
    memcpy(aBuffer, _ConvertedText, DynOS_String_Length(_ConvertedText) + 1);
}

//
// Data
//

struct DynosWarp {
/* 0 */ s16 mArea = 0;
/* 1 */ s16 mId = 0;
/* 2 */ s16 mType = -1;
/* 3 */ s16 mPosX = 0;
/* 4 */ s16 mPosY = 0;
/* 5 */ s16 mPosZ = 0;
/* 6 */ s16 mAngle = 0;
/* 7 */ s16 mDestLevel = 0;
/* 8 */ s16 mDestArea = 0;
/* 9 */ s16 mDestId = 0;
};

static void *sDynosLevelScripts[LEVEL_COUNT] = { NULL };
static Array<DynosWarp> sDynosLevelWarps[LEVEL_COUNT] = { Array<DynosWarp>() };
static Array<s32> sDynosLevelList = Array<s32>(); // Ordered by Course Id, COURSE_NONE excluded

static u64 DynOS_Level_CmdGet(void *aCmd, u64 aOffset) {
    u64 _Offset = (((aOffset) & 3llu) | (((aOffset) & ~3llu) << (sizeof(void *) >> 3llu)));
    return *((u64 *) (u64(aCmd) + _Offset));
}

static void *DynOS_Level_CmdNext(void *aCmd, u64 aCmdSize) {
    u64 _Offset = (((aCmdSize) & 3llu) | (((aCmdSize) & ~3llu) << (sizeof(void *) >> 3llu)));
    return (void *) (u64(aCmd) + _Offset);
}

static void DynOS_Level_ParseScript(const void *aScript, s32 (*aPreprocessFunction)(u8, void *));

//
// Init
//

static s32 DynOS_Level_PreprocessMasterScript(u8 aType, void *aCmd) {
    static bool sDynosScriptExecLevelTable = false;
    static s32 sDynosLevelNum = -1;
    
    if (!sDynosScriptExecLevelTable) {

        // JUMP_LINK
        if (aType == 0x06) {
            sDynosScriptExecLevelTable = true;
            return 0;
        }

    } else {

        // JUMP_IF
        if (aType == 0x0C) {
            sDynosLevelNum = (s32) DynOS_Level_CmdGet(aCmd, 0x04);
            return 0;
        }

        // EXECUTE
        if (aType == 0x00) {
            void *_Script = (void *) DynOS_Level_CmdGet(aCmd, 0x0C);
            if (sDynosLevelNum >= 0 && sDynosLevelNum < LEVEL_COUNT && !sDynosLevelScripts[sDynosLevelNum]) {
                sDynosLevelScripts[sDynosLevelNum] = _Script;
            }
            sDynosLevelNum = -1;
            return 2;
        }

        // EXIT
        if (aType == 0x02) {
            return 3;
        }

        // SLEEP
        if (aType == 0x03) {
            return 3;
        }
    }
    return 0;
}

static s32 sDynosCurrentLevelNum;
static s32 DynOS_Level_PreprocessScript(u8 aType, void *aCmd) {
    static u8 sDynosAreaIndex = 0;
    static auto _GetWarpStruct = [](u8 aArea, u8 aId) -> DynosWarp * {
        for (s32 i = 0; i != sDynosLevelWarps[sDynosCurrentLevelNum].Count(); ++i) {
            if (sDynosLevelWarps[sDynosCurrentLevelNum][i].mArea == aArea &&
                sDynosLevelWarps[sDynosCurrentLevelNum][i].mId == aId) {
                return &sDynosLevelWarps[sDynosCurrentLevelNum][i];
            }
        }
        DynosWarp _Warp;
        _Warp.mArea = aArea;
        _Warp.mId = aId;
        sDynosLevelWarps[sDynosCurrentLevelNum].Add(_Warp);
        return &sDynosLevelWarps[sDynosCurrentLevelNum][sDynosLevelWarps[sDynosCurrentLevelNum].Count() - 1];
    };

    // AREA
    if (aType == 0x1F) {
        sDynosAreaIndex = (u8) DynOS_Level_CmdGet(aCmd, 2);
    }

    // OBJECT
    else if (aType == 0x24) {
        const BehaviorScript *bhv = (const BehaviorScript *) DynOS_Level_CmdGet(aCmd, 20);
        for (s32 i = 0; i < 20; ++i) {
            if (sWarpBhvSpawnTable[i] == bhv) {
                DynosWarp *_Warp = _GetWarpStruct(sDynosAreaIndex, ((((u32) DynOS_Level_CmdGet(aCmd, 16)) >> 16) & 0xFF));
                if (_Warp->mType == -1) {
                    _Warp->mType = i;
                    _Warp->mPosX = (s16) DynOS_Level_CmdGet(aCmd, 4);
                    _Warp->mPosY = (s16) DynOS_Level_CmdGet(aCmd, 6);
                    _Warp->mPosZ = (s16) DynOS_Level_CmdGet(aCmd, 8);
                    _Warp->mAngle = (s16)((((s32)((s16) DynOS_Level_CmdGet(aCmd, 12))) * 0x8000) / 180);
                }
                break;
            }
        }
    }

    // WARP_NODE
    else if (aType == 0x26) {
        DynosWarp *_Warp = _GetWarpStruct(sDynosAreaIndex, (u8) DynOS_Level_CmdGet(aCmd, 2));
        if (_Warp->mDestLevel == 0) {
            _Warp->mDestLevel = (u8) DynOS_Level_CmdGet(aCmd, 3);
            _Warp->mDestArea = (u8) DynOS_Level_CmdGet(aCmd, 4);
            _Warp->mDestId = (u8) DynOS_Level_CmdGet(aCmd, 5);
        }
    }

    // PAINTING_WARP_NODE
    else if (aType == 0x27) {
        DynosWarp *_Warp = _GetWarpStruct(sDynosAreaIndex, (u8) DynOS_Level_CmdGet(aCmd, 2));
        if (_Warp->mDestLevel == 0) {
            _Warp->mDestLevel = (u8) DynOS_Level_CmdGet(aCmd, 3);
            _Warp->mDestArea = (u8) DynOS_Level_CmdGet(aCmd, 4);
            _Warp->mDestId = (u8) DynOS_Level_CmdGet(aCmd, 5);
        }
    }

    // SLEEP
    // SLEEP_BEFORE_EXIT
    else if (aType == 0x03 || aType == 0x04) {
        return 3;
    }

    return 0;
}

// Runs only once
static void DynOS_Level_Init() {
    static bool sInited = false;
    if (!sInited) {

        // Level scripts
        DynOS_Level_ParseScript(level_main_scripts_entry, DynOS_Level_PreprocessMasterScript);

        // Level warps
        for (sDynosCurrentLevelNum = 0; sDynosCurrentLevelNum != LEVEL_COUNT; ++sDynosCurrentLevelNum) {
            if (sDynosLevelScripts[sDynosCurrentLevelNum]) {
                DynOS_Level_ParseScript(sDynosLevelScripts[sDynosCurrentLevelNum], DynOS_Level_PreprocessScript);
            }
        }

        // Level list ordered by course id
        for (s32 i = COURSE_MIN; i <= COURSE_MAX; ++i) {
            if (i == COURSE_CAKE_END) continue;
            for (s32 j = 1; j != LEVEL_COUNT; ++j) {
                if (gLevelToCourseNumTable[j - 1] == i) {
                    sDynosLevelList.Add(j);
                }
            }
        }

        // Done
        sInited = true;
    }
}

//
// Common
//

s32 DynOS_Level_GetCount() {
    DynOS_Level_Init();
    return sDynosLevelList.Count();
}

const s32 *DynOS_Level_GetList() {
    DynOS_Level_Init();
    return sDynosLevelList.begin();
}

s32 DynOS_Level_GetCourse(s32 aLevel) {
    return (s32) gLevelToCourseNumTable[aLevel - 1];
}

const void *DynOS_Level_GetScript(s32 aLevel) {
    DynOS_Level_Init();
    return sDynosLevelScripts[aLevel];
}

//
// Course name
//

const u8 *DynOS_Level_GetName(s32 aLevel, bool aDecaps, bool aAddCourseNumber) {
    DynOS_Level_Init();
    static u8 sBuffer[256];
    memset(sBuffer, 0xFF, 256);
    s32 _Course = DynOS_Level_GetCourse(aLevel);

    // Level name
    if (aLevel == LEVEL_BOWSER_1) {
        SetConvertedTextToBuffer(sBuffer, DYNOS_LEVEL_TEXT_BOWSER_1);
    } else if (aLevel == LEVEL_BOWSER_2) {
        SetConvertedTextToBuffer(sBuffer, DYNOS_LEVEL_TEXT_BOWSER_2);
    } else if (aLevel == LEVEL_BOWSER_3) {
        SetConvertedTextToBuffer(sBuffer, DYNOS_LEVEL_TEXT_BOWSER_3);
    } else if (_Course < COURSE_BOB) {
        SetConvertedTextToBuffer(sBuffer, DYNOS_LEVEL_TEXT_CASTLE);
    } else if (_Course >= COURSE_CAKE_END) {
        SetConvertedTextToBuffer(sBuffer, DYNOS_LEVEL_TEXT_CASTLE);
    } else {
        const u8 *_CourseName = ((const u8 **) seg2_course_name_table)[_Course - COURSE_BOB] + 3;
        memcpy(sBuffer, _CourseName, DynOS_String_Length(_CourseName));
    }

    // Decaps
    if (aDecaps) {
        DynOS_String_Decapitalize(sBuffer);
    }

    // Course number
    if (aAddCourseNumber && (_Course >= COURSE_BOB) && (_Course <= COURSE_STAGES_MAX)) {
        memmove(sBuffer + 5, sBuffer, DynOS_String_Length(sBuffer));
        sBuffer[0] = ((_Course / 10) == 0 ? 158 : (_Course / 10));
        sBuffer[1] = (_Course % 10);
        sBuffer[2] = 158;
        sBuffer[3] = 159;
        sBuffer[4] = 158;
    }

    return sBuffer;
}

//
// Act/Star name
//

const u8 *DynOS_Level_GetActName(s32 aLevel, s32 aAct, bool aDecaps, bool aAddStarNumber) {
    DynOS_Level_Init();
    static u8 sBuffer[256];
    memset(sBuffer, 0xFF, 256);
    s32 _Course = DynOS_Level_GetCourse(aLevel);

    // Star name
    if (_Course < COURSE_BOB) {
        SetConvertedTextToBuffer(sBuffer, DYNOS_LEVEL_TEXT_ONE_SECRET_STAR);
    } else if (aLevel == LEVEL_BITDW) {
        SetConvertedTextToBuffer(sBuffer, DYNOS_LEVEL_TEXT_RED_COINS_STAR);
    } else if (aLevel == LEVEL_BITFS) {
        SetConvertedTextToBuffer(sBuffer, DYNOS_LEVEL_TEXT_RED_COINS_STAR);
    } else if (aLevel == LEVEL_BITS) {
        SetConvertedTextToBuffer(sBuffer, DYNOS_LEVEL_TEXT_RED_COINS_STAR);
    } else if (_Course > COURSE_STAGES_MAX) {
        SetConvertedTextToBuffer(sBuffer, DYNOS_LEVEL_TEXT_EMPTY);
    } else if (aAct >= 7) {
        SetConvertedTextToBuffer(sBuffer, DYNOS_LEVEL_TEXT_100_COINS_STAR);
    } else {
        const u8 *_ActName = ((const u8 **) seg2_act_name_table)[(_Course - COURSE_BOB) * 6 + (aAct - 1)];
        memcpy(sBuffer, _ActName, DynOS_String_Length(_ActName));
    }

    // Decaps
    if (aDecaps) {
        DynOS_String_Decapitalize(sBuffer);
    }

    // Star number
    if (aAddStarNumber && (_Course >= COURSE_BOB) && (_Course <= COURSE_STAGES_MAX)) {
        memmove(sBuffer + 5, sBuffer, DynOS_String_Length(sBuffer));
        sBuffer[0] = ((aAct / 10) == 0 ? 158 : (aAct / 10));
        sBuffer[1] = (aAct % 10);
        sBuffer[2] = 158;
        sBuffer[3] = 159;
        sBuffer[4] = 158;
    }

    return sBuffer;
}

const u8 *DynOS_Level_GetAreaName(s32 aLevel, s32 aArea, bool aDecaps) {
    DynOS_Level_Init();
    static const char *sAreaNamesPerLevel[][4] = {
        { "", "", "", "" },
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* BoB */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* WF */
        { "MAIN AREA", "SUNKEN SHIP", "NOT AVAILABLE", "NOT AVAILABLE" }, /* JRB */
        { "MAIN AREA", "COTTAGE SLIDE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* CCM */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* BBH */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* HMC */
        { "MAIN AREA", "VOLCANO", "NOT AVAILABLE", "NOT AVAILABLE" }, /* LLL */
        { "MAIN AREA", "PYRAMID", "EYEROCK'S ROOM", "NOT AVAILABLE" }, /* SSL */
        { "MAIN AREA", "DOCKS", "NOT AVAILABLE", "NOT AVAILABLE" }, /* DDD */
        { "MAIN AREA", "IGLOO", "NOT AVAILABLE", "NOT AVAILABLE" }, /* SL */
        { "MAIN AREA", "DOWNTOWN", "NOT AVAILABLE", "NOT AVAILABLE" }, /* WDW */
        { "MAIN AREA", "SECRET SLIDE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* TTM */
        { "HUGE ISLAND", "TINY ISLAND", "WIGGLER'S ROOM", "NOT AVAILABLE" }, /* THI */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* TTC */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* RR */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* BITDW */
        { "BOWSER BATTLE", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* Bowser 1 */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* BITFS */
        { "BOWSER BATTLE", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* Bowser 2 */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* BITS */
        { "BOWSER BATTLE", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* Bowser 3 */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* PSS */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* TOTWC */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* COTMC */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* VCUTM */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* WMOTR */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* SA */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* Castle grounds */
        { "FIRST FLOOR", "SECOND FLOOR", "BASEMENT", "NOT AVAILABLE" }, /* Castle inside */
        { "MAIN AREA", "NOT AVAILABLE", "NOT AVAILABLE", "NOT AVAILABLE" }, /* Castle courtyard */
    };
    static u8 sBuffer[256];
    memset(sBuffer, 0xFF, 256);

    // Area name
    switch (aLevel) {
        case LEVEL_BOB: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[1][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_WF: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[2][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_JRB: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[3][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_CCM: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[4][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_BBH: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[5][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_HMC: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[6][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_LLL: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[7][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_SSL: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[8][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_DDD: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[9][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_SL: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[10][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_WDW: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[11][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_TTM: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[12][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_THI: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[13][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_TTC: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[14][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_RR: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[15][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_BITDW: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[16][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_BOWSER_1: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[17][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_BITFS: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[18][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_BOWSER_2: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[19][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_BITS: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[20][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_BOWSER_3: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[21][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_PSS: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[22][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_TOTWC: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[23][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_COTMC: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[24][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_VCUTM: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[25][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_WMOTR: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[26][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_SA: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[27][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_CASTLE_GROUNDS: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[28][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_CASTLE: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[29][MIN(MAX(aArea - 1, 0), 3)]); break;
        case LEVEL_CASTLE_COURTYARD: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[30][MIN(MAX(aArea - 1, 0), 3)]); break;
        default: SetConvertedTextToBuffer(sBuffer, sAreaNamesPerLevel[0][MIN(MAX(aArea - 1, 0), 3)]); break;
    }

    // Decaps
    if (aDecaps) {
        DynOS_String_Decapitalize(sBuffer);
    }
    return sBuffer;
}

//
// Level Script Preprocessing
// By default,
// - Ifs are always true
// - Skips are always false
// - Loops break after the first loop
//

struct LvlCmd {
    u8 mType;
    u8 mSize;
};

struct Stack {
    u64 mData[32];
    s32 mBaseIndex;
    s32 mTopIndex;
};

template <typename T>
static void StackPush(Stack& aStack, const T &aValue) {
    if (aStack.mTopIndex >= 0) {
        aStack.mData[aStack.mTopIndex] = u64(aValue);
        aStack.mTopIndex++;
    }
}

template <typename T>
static T StackPop(Stack& aStack) {
    if (aStack.mTopIndex <= 0) {
        return (T) 0;
    }
    aStack.mTopIndex--;
    return (T) aStack.mData[aStack.mTopIndex];
}

static LvlCmd *DynOS_Level_CmdExecute(Stack &aStack, LvlCmd *aCmd) {
    StackPush(aStack, DynOS_Level_CmdNext(aCmd, aCmd->mSize));
    StackPush(aStack, aStack.mBaseIndex);
    aStack.mBaseIndex = aStack.mTopIndex;
    return (LvlCmd *) DynOS_Level_CmdGet(aCmd, 12);
}

static LvlCmd *DynOS_Level_CmdExitAndExecute(Stack &aStack, LvlCmd *aCmd) {
    aStack.mTopIndex = aStack.mBaseIndex;
    return (LvlCmd *) DynOS_Level_CmdGet(aCmd, 12);
}

static LvlCmd *DynOS_Level_CmdExit(Stack &aStack, LvlCmd *aCmd) {
    aStack.mTopIndex = aStack.mBaseIndex;
    aStack.mBaseIndex = StackPop<s32>(aStack);
    return StackPop<LvlCmd *>(aStack);
}

static LvlCmd *DynOS_Level_CmdSleep(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSleepBeforeExit(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdJump(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdGet(aCmd, 4);
}

static LvlCmd *DynOS_Level_CmdJumpLink(Stack &aStack, LvlCmd *aCmd) {
    StackPush(aStack, DynOS_Level_CmdNext(aCmd, aCmd->mSize));
    return (LvlCmd *) DynOS_Level_CmdGet(aCmd, 4);
}

static LvlCmd *DynOS_Level_CmdReturn(Stack &aStack, UNUSED LvlCmd *aCmd) {
    return StackPop<LvlCmd *>(aStack);
}

static LvlCmd *DynOS_Level_CmdJumpLinkPushArg(Stack &aStack, LvlCmd *aCmd) {
    StackPush(aStack, DynOS_Level_CmdNext(aCmd, aCmd->mSize));
    StackPush(aStack, DynOS_Level_CmdGet(aCmd, 2));
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdJumpRepeat(Stack &aStack, LvlCmd *aCmd) {
    aStack.mTopIndex -= 2;
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdLoopBegin(Stack &aStack, LvlCmd *aCmd) {
    StackPush(aStack, DynOS_Level_CmdNext(aCmd, aCmd->mSize));
    StackPush(aStack, 0);
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdLoopUntil(Stack &aStack, LvlCmd *aCmd) {
    aStack.mTopIndex -= 2;
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdJumpIf(Stack &aStack, LvlCmd *aCmd) {
    StackPush(aStack, DynOS_Level_CmdNext(aCmd, aCmd->mSize)); /* Not an error, that's intentional */
    return (LvlCmd *) DynOS_Level_CmdGet(aCmd, 8);
}

static LvlCmd *DynOS_Level_CmdJumpLinkIf(Stack &aStack, LvlCmd *aCmd) {
    StackPush(aStack, DynOS_Level_CmdNext(aCmd, aCmd->mSize));
    return (LvlCmd *) DynOS_Level_CmdGet(aCmd, 8);
}

static LvlCmd *DynOS_Level_CmdSkipIf(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSkip(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSkipNop(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdCall(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdCallLoop(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSetRegister(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdPushPool(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdPopPool(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdLoadFixed(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdLoadRaw(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdLoadMIO0(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdLoadMarioHead(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdLoadMIO0Texture(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdInitLevel(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdClearLevel(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdAllocLevelPool(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdFreeLevelPool(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdBeginArea(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdEndArea(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdLoadModelFromDL(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdLoadModelFromGeo(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_Cmd23(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdMario(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdObject(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdWarpNode(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdInstantWarp(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSetTerrainType(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdPaintingWarpNode(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_Cmd3A(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSetWhirlpool(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSetBlackout(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSetGamma(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSetTerrain(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSetRooms(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdMacroObjects(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdLoadArea(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdUnloadArea(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSetMarioStartPos(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_Cmd2C(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_Cmd2D(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSetTransition(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdNop(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdShowDialog(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSetBackgroundMusic(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdSetMenuMusic(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdStopMusic(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdGetOrSet(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdAdvanceDemo(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdClearDemoPointer(Stack &aStack, LvlCmd *aCmd) {
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static LvlCmd *DynOS_Level_CmdJumpArea(Stack &aStack, LvlCmd *aCmd, s32 (*aPreprocessFunction)(u8, void *)) {
    DynOS_Level_ParseScript((const void *) DynOS_Level_CmdGet(aCmd, 8), aPreprocessFunction);
    return (LvlCmd *) DynOS_Level_CmdNext(aCmd, aCmd->mSize);
}

static void DynOS_Level_ParseScript(const void *aScript, s32 (*aPreprocessFunction)(u8, void *)) {
    Stack _Stack;
    _Stack.mBaseIndex = -1;
    _Stack.mTopIndex = 0;
    for (LvlCmd *_Cmd = (LvlCmd *) aScript; _Cmd != NULL;) {
        u8 _CmdType = (_Cmd->mType & 0x3F);
        s32 _Action = aPreprocessFunction(_CmdType, (void *) _Cmd);
        switch (_Action) {
            case 0:
                switch (_CmdType) {
                    case 0x00: _Cmd = DynOS_Level_CmdExecute(_Stack, _Cmd); break;
                    case 0x01: _Cmd = DynOS_Level_CmdExitAndExecute(_Stack, _Cmd); break;
                    case 0x02: _Cmd = DynOS_Level_CmdExit(_Stack, _Cmd); break;
                    case 0x03: _Cmd = DynOS_Level_CmdSleep(_Stack, _Cmd); break;
                    case 0x04: _Cmd = DynOS_Level_CmdSleepBeforeExit(_Stack, _Cmd); break;
                    case 0x05: _Cmd = DynOS_Level_CmdJump(_Stack, _Cmd); break;
                    case 0x06: _Cmd = DynOS_Level_CmdJumpLink(_Stack, _Cmd); break;
                    case 0x07: _Cmd = DynOS_Level_CmdReturn(_Stack, _Cmd); break;
                    case 0x08: _Cmd = DynOS_Level_CmdJumpLinkPushArg(_Stack, _Cmd); break;
                    case 0x09: _Cmd = DynOS_Level_CmdJumpRepeat(_Stack, _Cmd); break;
                    case 0x0A: _Cmd = DynOS_Level_CmdLoopBegin(_Stack, _Cmd); break;
                    case 0x0B: _Cmd = DynOS_Level_CmdLoopUntil(_Stack, _Cmd); break;
                    case 0x0C: _Cmd = DynOS_Level_CmdJumpIf(_Stack, _Cmd); break;
                    case 0x0D: _Cmd = DynOS_Level_CmdJumpLinkIf(_Stack, _Cmd); break;
                    case 0x0E: _Cmd = DynOS_Level_CmdSkipIf(_Stack, _Cmd); break;
                    case 0x0F: _Cmd = DynOS_Level_CmdSkip(_Stack, _Cmd); break;
                    case 0x10: _Cmd = DynOS_Level_CmdSkipNop(_Stack, _Cmd); break;
                    case 0x11: _Cmd = DynOS_Level_CmdCall(_Stack, _Cmd); break;
                    case 0x12: _Cmd = DynOS_Level_CmdCallLoop(_Stack, _Cmd); break;
                    case 0x13: _Cmd = DynOS_Level_CmdSetRegister(_Stack, _Cmd); break;
                    case 0x14: _Cmd = DynOS_Level_CmdPushPool(_Stack, _Cmd); break;
                    case 0x15: _Cmd = DynOS_Level_CmdPopPool(_Stack, _Cmd); break;
                    case 0x16: _Cmd = DynOS_Level_CmdLoadFixed(_Stack, _Cmd); break;
                    case 0x17: _Cmd = DynOS_Level_CmdLoadRaw(_Stack, _Cmd); break;
                    case 0x18: _Cmd = DynOS_Level_CmdLoadMIO0(_Stack, _Cmd); break;
                    case 0x19: _Cmd = DynOS_Level_CmdLoadMarioHead(_Stack, _Cmd); break;
                    case 0x1A: _Cmd = DynOS_Level_CmdLoadMIO0Texture(_Stack, _Cmd); break;
                    case 0x1B: _Cmd = DynOS_Level_CmdInitLevel(_Stack, _Cmd); break;
                    case 0x1C: _Cmd = DynOS_Level_CmdClearLevel(_Stack, _Cmd); break;
                    case 0x1D: _Cmd = DynOS_Level_CmdAllocLevelPool(_Stack, _Cmd); break;
                    case 0x1E: _Cmd = DynOS_Level_CmdFreeLevelPool(_Stack, _Cmd); break;
                    case 0x1F: _Cmd = DynOS_Level_CmdBeginArea(_Stack, _Cmd); break;
                    case 0x20: _Cmd = DynOS_Level_CmdEndArea(_Stack, _Cmd); break;
                    case 0x21: _Cmd = DynOS_Level_CmdLoadModelFromDL(_Stack, _Cmd); break;
                    case 0x22: _Cmd = DynOS_Level_CmdLoadModelFromGeo(_Stack, _Cmd); break;
                    case 0x23: _Cmd = DynOS_Level_Cmd23(_Stack, _Cmd); break;
                    case 0x24: _Cmd = DynOS_Level_CmdObject(_Stack, _Cmd); break;
                    case 0x25: _Cmd = DynOS_Level_CmdMario(_Stack, _Cmd); break;
                    case 0x26: _Cmd = DynOS_Level_CmdWarpNode(_Stack, _Cmd); break;
                    case 0x27: _Cmd = DynOS_Level_CmdPaintingWarpNode(_Stack, _Cmd); break;
                    case 0x28: _Cmd = DynOS_Level_CmdInstantWarp(_Stack, _Cmd); break;
                    case 0x29: _Cmd = DynOS_Level_CmdLoadArea(_Stack, _Cmd); break;
                    case 0x2A: _Cmd = DynOS_Level_CmdUnloadArea(_Stack, _Cmd); break;
                    case 0x2B: _Cmd = DynOS_Level_CmdSetMarioStartPos(_Stack, _Cmd); break;
                    case 0x2C: _Cmd = DynOS_Level_Cmd2C(_Stack, _Cmd); break;
                    case 0x2D: _Cmd = DynOS_Level_Cmd2D(_Stack, _Cmd); break;
                    case 0x2E: _Cmd = DynOS_Level_CmdSetTerrain(_Stack, _Cmd); break;
                    case 0x2F: _Cmd = DynOS_Level_CmdSetRooms(_Stack, _Cmd); break;
                    case 0x30: _Cmd = DynOS_Level_CmdShowDialog(_Stack, _Cmd); break;
                    case 0x31: _Cmd = DynOS_Level_CmdSetTerrainType(_Stack, _Cmd); break;
                    case 0x32: _Cmd = DynOS_Level_CmdNop(_Stack, _Cmd); break;
                    case 0x33: _Cmd = DynOS_Level_CmdSetTransition(_Stack, _Cmd); break;
                    case 0x34: _Cmd = DynOS_Level_CmdSetBlackout(_Stack, _Cmd); break;
                    case 0x35: _Cmd = DynOS_Level_CmdSetGamma(_Stack, _Cmd); break;
                    case 0x36: _Cmd = DynOS_Level_CmdSetBackgroundMusic(_Stack, _Cmd); break;
                    case 0x37: _Cmd = DynOS_Level_CmdSetMenuMusic(_Stack, _Cmd); break;
                    case 0x38: _Cmd = DynOS_Level_CmdStopMusic(_Stack, _Cmd); break;
                    case 0x39: _Cmd = DynOS_Level_CmdMacroObjects(_Stack, _Cmd); break;
                    case 0x3A: _Cmd = DynOS_Level_Cmd3A(_Stack, _Cmd); break;
                    case 0x3B: _Cmd = DynOS_Level_CmdSetWhirlpool(_Stack, _Cmd); break;
                    case 0x3C: _Cmd = DynOS_Level_CmdGetOrSet(_Stack, _Cmd); break;
                    case 0x3D: _Cmd = DynOS_Level_CmdAdvanceDemo(_Stack, _Cmd); break;
                    case 0x3E: _Cmd = DynOS_Level_CmdClearDemoPointer(_Stack, _Cmd); break;
                    case 0x3F: _Cmd = DynOS_Level_CmdJumpArea(_Stack, _Cmd, aPreprocessFunction); break;
                } break;

            case 1:
                _Cmd = (LvlCmd *) DynOS_Level_CmdNext(_Cmd, _Cmd->mSize);
                break;

            case 2:
                _Cmd = DynOS_Level_CmdReturn(_Stack, _Cmd);
                break;

            case 3:
                return;
        }
    }
}

//
// Level Script Utilities
//

s16 *DynOS_Level_GetWarp(s32 aLevel, s32 aArea, u8 aWarpId) {
    DynOS_Level_Init();
    for (const auto &_Warp : sDynosLevelWarps[aLevel]) {
        if (_Warp.mArea == aArea && _Warp.mId == aWarpId) {
            return (s16 *) &_Warp;
        }
    }
    return NULL;
}

s16 *DynOS_Level_GetWarpEntry(s32 aLevel, s32 aArea) {
    DynOS_Level_Init();
    if (aLevel == LEVEL_TTM && aArea > 2) return NULL;
    return DynOS_Level_GetWarp(aLevel, aArea, 0x0A);
}

s16 *DynOS_Level_GetWarpDeath(s32 aLevel, s32 aArea) {
    DynOS_Level_Init();
    s16 *_Warp = DynOS_Level_GetWarp(aLevel, aArea, 0xF1);
    if (!_Warp) _Warp = DynOS_Level_GetWarp(aLevel, aArea, 0xF3);
    return _Warp;
}

