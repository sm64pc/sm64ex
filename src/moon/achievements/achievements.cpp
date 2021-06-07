#include "achievements.h"
#include "moon/mod-engine/hooks/hook.h"
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/interfaces/moon-screen.h"
#include "moon/utils/umath.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include "moon/ui/interfaces/moon-screen.h"
#include "moon/ui/animation/algorithms.h"
#include "moon/mod-engine/models/mod-model.h"

extern "C" {
#include "pc/cheats.h"
#include "moon/utils/moon-gfx.h"
#include "pc/platform.h"
#include "game/level_update.h"
#include "audio/external.h"
#include "audio_defines.h"
#include "game/area.h"
#include "game/object_list_processor.h"
#include "sm64.h"
#include "level_table.h"
#include "game/save_file.h"
}

#define ALL_MAIN_STARS 0x7F

using namespace std;

map<string, Achievement*> registeredAchievements;
vector<AchievementEntry*> entries;
bool cheatsGotEnabled = false;

namespace AchievementList {
    /* Star achievements */
    Achievement* GET_1_STAR         = MoonAchievements::bind(new Achievement("achievement.get1Stars",   "textures/moon/achievements/stars.1.rgba16",  "Your journey begins!",     "Get one star",  false, 0, 150, nullptr));
    Achievement* GET_8_STARS        = MoonAchievements::bind(new Achievement("achievement.get8Stars",   "textures/moon/achievements/stars.8.rgba16",  "You feel a strong power", "Get 8 stars",    false, 0, 150, GET_1_STAR));
    Achievement* GET_30_STARS       = MoonAchievements::bind(new Achievement("achievement.get30Stars",  "textures/moon/achievements/stars.30.rgba16", "Earning A Quarter",       "Get 30 stars",   false, 0, 150, GET_8_STARS));
    Achievement* GET_31_STARS       = MoonAchievements::bind(new Achievement("achievement.get31Stars",  "textures/moon/achievements/stars.31.rgba16", "Extra Cent",              "Get 31 stars",   false, 0, 150, GET_8_STARS));
    Achievement* GET_50_STARS       = MoonAchievements::bind(new Achievement("achievement.get50Stars",  "textures/moon/achievements/stars.50.rgba16", "Lucky Eight",             "Get 50 stars",   false, 0, 150, GET_30_STARS));
    Achievement* GET_70_STARS       = MoonAchievements::bind(new Achievement("achievement.get70Stars",  "textures/moon/achievements/stars.70.rgba16", "Halfway done!",           "Get 70 stars",   false, 0, 150, GET_50_STARS));
    Achievement* GET_120_STARS      = MoonAchievements::bind(new Achievement("achievement.get120Stars", "textures/moon/achievements/stars.120.rgba16", "The Completionist",      "Get 120 stars",  false, 0, 150, GET_70_STARS));

    /* Cap Achievements */
    Achievement* UNLOCK_WING_CAP    = MoonAchievements::bind(new Achievement("achievement.unlockWingCap",   "textures/moon/achievements/caps.wing-cap.rgba16",   "Super Man-rio",      "Unlock the wing cap",   false, 0, 150, nullptr));
    Achievement* UNL0CK_METAL_CAP   = MoonAchievements::bind(new Achievement("achievement.unlockMetalCap",  "textures/moon/achievements/caps.metal-cap.rgba16",  "Heavy-Headed",       "Unlock the metal cap",  false, 0, 150, nullptr));
    Achievement* UNLOCK_VANISH_CAP  = MoonAchievements::bind(new Achievement("achievement.unlockVanishCap", "textures/moon/achievements/caps.vanish-cap.rgba16", "Wait, Where Is He?", "Unlock the vanish cap", false, 0, 150, nullptr));

    /* Level Achievements */
    Achievement* GET_6_LEVEL_STARS  = MoonAchievements::bind(new Achievement("achievement.get6MainStars",         "textures/moon/achievements/ranks.f.rgba16", "F Rank",  "Get all 6 Main Stars in One Level",      false, 0, 150, nullptr));
    Achievement* GET_100_COIN_STAR  = MoonAchievements::bind(new Achievement("achievement.get100CoinStar",        "textures/moon/achievements/ranks.e.rgba16", "E Rank",  "Get a 100 Coin Star in One Level",       false, 0, 150, nullptr));
    Achievement* GET_ALL_LVL_COINS  = MoonAchievements::bind(new Achievement("achievement.getAllCoins",           "textures/moon/achievements/ranks.d.rgba16", "D Rank",  "Get all Coins in One Level",             false, 0, 150, nullptr));
    Achievement* GET_FLOOR_0_STARS  = MoonAchievements::bind(new Achievement("achievement.getAllStarsInBasement", "textures/moon/achievements/ranks.b.rgba16", "B Rank",  "Get all Main Stars in the Basement",     false, 0, 150, nullptr));
    Achievement* GET_FLOOR_1_STARS  = MoonAchievements::bind(new Achievement("achievement.getAllStarsInFloor1",   "textures/moon/achievements/ranks.c.rgba16", "C Rank",  "Get all Main Stars in the First Floor",  false, 0, 150, nullptr));
    Achievement* GET_FLOOR_2_STARS  = MoonAchievements::bind(new Achievement("achievement.getAllStarsInFloor2",   "textures/moon/achievements/ranks.a.rgba16", "A Rank",  "Get all Main Stars in the Second Floor", false, 0, 150, nullptr));
    Achievement* GET_FLOOR_3_STARS  = MoonAchievements::bind(new Achievement("achievement.getAllStarsInFloor3",   "textures/moon/achievements/ranks.s.rgba16", "S Rank",  "Get all Main Stars in the Third Floor",  false, 0, 150, nullptr));
    Achievement* GET_CASTLE_STARS   = MoonAchievements::bind(new Achievement("achievement.getAllCastleStars",     "textures/moon/achievements/ranks.splus.rgba16", "S+ Rank", "Get all Castle Secret Stars",        false, 0, 150, nullptr));

    /* Boss Achievements */
    Achievement* DEFEAT_MRI         = MoonAchievements::bind(new Achievement("achievement.beatMr.I",          "textures/moon/achievements/bosses.mr-i.rgba16", "I vs Eye",                  "Beat Mr.I",                        false, 0, 150, nullptr));
    Achievement* DEFEAT_WIGGLER     = MoonAchievements::bind(new Achievement("achievement.beatWiggler",       "textures/moon/achievements/bosses.wiggler.rgba16", "Insecticude",            "Beat Wiggler",                     false, 0, 150, nullptr));
    Achievement* DEFEAT_KINGBOB     = MoonAchievements::bind(new Achievement("achievement.beatKingBobOmb",    "textures/moon/achievements/bosses.big-bob.rgba16", "Explosive Test",         "Beat King Bob-Omb",                false, 0, 150, nullptr));
    Achievement* DEFEAT_ALL_BULLY   = MoonAchievements::bind(new Achievement("achievement.beatAllBigBullies", "textures/moon/achievements/bosses.big-bully.rgba16", "The Real Bully",       "Beat all big bullies from LLL",    false, 0, 150, nullptr));
    Achievement* DEFEAT_EYEROK      = MoonAchievements::bind(new Achievement("achievement.beatEyerok",        "textures/moon/achievements/bosses.eyerok.rgba16", "Welcome To The Jam",      "Beat Eyerok",                      false, 0, 150, nullptr));
    Achievement* DEFEAT_KINGWHOMP   = MoonAchievements::bind(new Achievement("achievement.beatKingWhomp",     "textures/moon/achievements/bosses.king-whomp.rgba16", "Come On And Slam",    "Beat King Whomp",                  false, 0, 150, nullptr));
    Achievement* DEFEAT_BOWSER2     = MoonAchievements::bind(new Achievement("achievement.beatSecondBowser",  "textures/moon/achievements/bowser.2.rgba16", "Bowser Burned By The Lava",    "Beat second Bowser",               false, 0, 150, nullptr));
    Achievement* DEFEAT_BOWSER1     = MoonAchievements::bind(new Achievement("achievement.beatFirstBowser",   "textures/moon/achievements/bowser.1.rgba16", "Bowser Trapped In The Dark",   "Beat first Bowser",                false, 0, 150, nullptr));
    Achievement* DEFEAT_120S_BOWSER = MoonAchievements::bind(new Achievement("achievement.beatGame120Stars",  "textures/moon/achievements/bowser.3-with-120-stars.rgba16", "Power Battle",  "Beat final Bowser with 120 stars", false, 0, 150, nullptr));
    Achievement* DEFEAT_ALL_BOOS    = MoonAchievements::bind(new Achievement("achievement.beatAll3BoosOnLLL", "textures/moon/achievements/bosses.big-boo.rgba16", "Mario, Where Are You??", "Beat all Boos from BBH",         false, 0, 150, nullptr));
    Achievement* DEFEAT_BOWSER3     = MoonAchievements::bind(new Achievement("achievement.beatFinalBowser",   "textures/moon/achievements/bowser.3.rgba16", "Bowser Launched Into The Sky", "Beat final Bowser",                false, 0, 150, nullptr));

    /* Death Achievements */
    Achievement* DEATH_BY_BOSS      = MoonAchievements::bind(new Achievement("achievement.deathByBoss",       "textures/moon/achievements/deaths.boss.rgba16", "Git Gud",              "Get killed by a boss",         false, 0, 150, nullptr));
    Achievement* DEATH_BY_FALLING   = MoonAchievements::bind(new Achievement("achievement.deathByFalling",    "textures/moon/achievements/deaths.falling.rgba16", "My Leg!",           "Die by falling",               false, 0, 150, nullptr));
    Achievement* DEATH_BY_SAND      = MoonAchievements::bind(new Achievement("achievement.deathBySand",       "textures/moon/achievements/deaths.quicksand.rgba16", "Sinreked",          "Die by sinking sand",          false, 0, 150, nullptr));
    Achievement* DEATH_BY_CRUSHING  = MoonAchievements::bind(new Achievement("achievement.deathByCrushing",   "textures/moon/achievements/deaths.crushed.rgba16", "Space Jam",         "Get crushed",                  false, 0, 150, nullptr));
    Achievement* DEATH_BY_BOWSER    = MoonAchievements::bind(new Achievement("achievement.deathByBowser",     "textures/moon/achievements/deaths.bowser.rgba16", "Bad Ending",         "Get killed by Bowser",         false, 0, 150, nullptr));
    Achievement* DEATH_BY_ENEMY     = MoonAchievements::bind(new Achievement("achievement.deathByEnemy",      "textures/moon/achievements/deaths.standard.rgba16", "Classic Way",      "Get killed by a normal enemy", false, 0, 150, nullptr));
    Achievement* DEATH_BY_DROWNING  = MoonAchievements::bind(new Achievement("achievement.deathByDrowning",   "textures/moon/achievements/deaths.drowning.rgba16", "Under The Sea",    "Die by drowning",              false, 0, 150, nullptr));
    Achievement* DEATH_BY_FIRE      = MoonAchievements::bind(new Achievement("achievement.deathByFire",       "textures/moon/achievements/extras.carpet-burn.rgba16", "Roasted Mario", "Die by fire",                  false, 0, 150, nullptr));

    /* Extra Achievements */
    Achievement* TALK_WITH_YOSHI     = MoonAchievements::bind(new Achievement("achievement.talkWithYoshi",     "textures/moon/achievements/extras.yoshi.rgba16",       "It Is You?",           "Talk with Yoshi",            false, 0, 150, nullptr));
    Achievement* SLIDE_100_TIMES     = MoonAchievements::bind(new Achievement("achievement.slide20Times",      "textures/moon/achievements/extras.carpet-burn.rgba16", "Burned Ass",           "Go trough slides 100 times", false, 0, 150, nullptr));
//  Achievement* BEAT_EVERY_RACE     = MoonAchievements::bind(new Achievement("achievement.beatEveryRace",     "textures/moon/achievements/extras.runner.rgba16",      "Olympic Runner",       "Beat every race",            false, 0, 150, nullptr));
    Achievement* TALK_25_TIMES       = MoonAchievements::bind(new Achievement("achievement.talk25Times",       "textures/moon/achievements/extras.talker.rgba16",      "Olympic Talker",       "Talk 25 times with npcs",    false, 0, 150, nullptr));
    Achievement* JUMP_1000_TIMES     = MoonAchievements::bind(new Achievement("achievement.jump1000Times",     "textures/moon/achievements/extras.swimmer.rgba16",     "Olympic Swimmer",      "Grab every star that needs Metal Cap without it", false, 0, 150, nullptr));
    Achievement* WATCH_END_CREDITS   = MoonAchievements::bind(new Achievement("achievement.watchEndCredits",   "textures/moon/achievements/extras.cake.rgba16",        "The Cake Is A Lie?!",  "Watch the end credits",      false, 0, 150, nullptr));
    Achievement* RELEASE_CHAIN_CHOMP = MoonAchievements::bind(new Achievement("achievement.releaseChainChomp", "textures/moon/achievements/extras.chain-chomp.rgba16", "Who Let The Dog Out?", "Get killed by a boss",       false, 0, 150, nullptr));

    Achievement* CHEATER             = MoonAchievements::bind(new Achievement("achievement.cheater",   "mod-icons://Moon64",  "What a loser!", "You turned on cheats", false, 0, 150, nullptr));
};

int nId = 0;

namespace MoonAchievements {
    Achievement* bind(Achievement* achievement){
        achievement->sortId = nId;
        registeredAchievements[achievement->id] = achievement;
        nId++;
        return achievement;
    }
}

map<int, int> levelCoins = {
    { LEVEL_BOB, 146 },
    { LEVEL_WF,  141 },
    { LEVEL_JRB, 104 },
    { LEVEL_CCM, 154 },
    { LEVEL_BBH, 151 },
    { LEVEL_HMC, 134 },
    { LEVEL_LLL, 133 },
    { LEVEL_SSL, 136 },
    { LEVEL_DDD, 106 },
    { LEVEL_SL,  125 },
    { LEVEL_WDW, 147 },
    { LEVEL_TTM, 135 },
    { LEVEL_THI, 182 },
    { LEVEL_TTC, 128 },
    { LEVEL_RR,  146 }
};

#define NEXT_FLOOR -1

vector<int> castleStars = {
    LEVEL_SSL,
    LEVEL_LLL,
    LEVEL_HMC,
    LEVEL_DDD,
    NEXT_FLOOR,
    LEVEL_BOB,
    LEVEL_CCM,
    LEVEL_BBH,
    LEVEL_WF,
    LEVEL_JRB,
    NEXT_FLOOR,
    LEVEL_THI,
    LEVEL_WDW,
    LEVEL_TTM,
    LEVEL_SL,
    NEXT_FLOOR,
    LEVEL_TTC,
    LEVEL_RR,
    NEXT_FLOOR,
};

namespace MoonTriggers {

    int jumpCount = 0;
    int slideCount = 0;
    int npcTalk = 0;

    bool executed = false;

    int getObjectsAmount(vector<int> ids){
        int count = 0;
        for (s32 i = 0; i != NUM_OBJ_LISTS; ++i) {
            struct ObjectNode *listHead = &gObjectLists[i];
            struct Object *next = (struct Object *) listHead->next;
            while (next != (struct Object *) listHead) {
                int id = Moon::GetGraphNodeID(next->header.gfx.sharedChild);
                if( find(ids.begin(), ids.end(), id) != ids.end())
                    count++;
                next = (struct Object *) next->header.next;
            }
        }
        return count;
    }

    void actionTriggers(){
        switch(gMarioState->action){
            case ACT_BUTT_SLIDE:
                if(!executed)
                    slideCount++;
                executed = true;
                break;
            case ACT_JUMP:
            case ACT_JUMP_KICK:
            case ACT_DOUBLE_JUMP:
            case ACT_TRIPLE_JUMP:
            case ACT_LONG_JUMP:
            case ACT_JUMP_LAND:
                if(!executed)
                    jumpCount++;
                executed = true;
                break;
            case ACT_READING_NPC_DIALOG:
                if(!executed)
                    npcTalk++;
                executed = true;
                break;
            default:
                executed = false;
        }

        if(slideCount >= 100)
            Moon::showAchievement(AchievementList::SLIDE_100_TIMES);
        if(jumpCount >= 1000)
            Moon::showAchievement(AchievementList::JUMP_1000_TIMES);
        if(npcTalk >= 25)
            Moon::showAchievement(AchievementList::TALK_25_TIMES);
    }

    void deathTrigger(){
        if( gMarioState->health < 0x0100 ){
            if(is_playing(0x16)){
                Moon::showAchievement(AchievementList::DEATH_BY_BOSS);
                return;
            }
            if(is_playing(0x07) || is_playing(0x19)){
                Moon::showAchievement(AchievementList::DEATH_BY_BOWSER);
                return;
            }
        }
    }

    void starsTrigger(){
        if( levelCoins.find(gCurrLevelNum) != levelCoins.end() && gHudDisplay.coins >= levelCoins[gCurrLevelNum])
            Moon::showAchievement(AchievementList::GET_ALL_LVL_COINS);


        if(gCurrLevelNum == LEVEL_LLL && gCurrAreaIndex != 2){
            if(getObjectsAmount({0x56, 0x57}) == 0)
                Moon::showAchievement(AchievementList::DEFEAT_ALL_BULLY);
        }

        if(gCurrLevelNum == LEVEL_BBH){
            if(getObjectsAmount({0x54}) == 0)
                Moon::showAchievement(AchievementList::DEFEAT_ALL_BOOS);
        }

        bool allObtainedStars = false;
        int currentFloor = 0;
        Achievement* castleAchievements[] = {
            AchievementList::GET_FLOOR_0_STARS,
            AchievementList::GET_FLOOR_1_STARS,
            AchievementList::GET_FLOOR_2_STARS,
            AchievementList::GET_FLOOR_3_STARS
        };

        for( auto &star : castleStars){
            if(star == NEXT_FLOOR){
                if(allObtainedStars)
                    Moon::showAchievement(castleAchievements[currentFloor]);
                currentFloor++;
                continue;
            }

            s32 stars = save_file_get_star_flags(gCurrSaveFileNum - 1, star - 1);
            if(stars > 6){
                Moon::showAchievement(AchievementList::GET_6_LEVEL_STARS);
                allObtainedStars = true;
            } else {
                allObtainedStars = false;
            }
        }

        s32 stars = save_file_get_star_flags(gCurrSaveFileNum - 1, -1);
        if(stars >= 5)
            Moon::showAchievement(AchievementList::GET_CASTLE_STARS);

        Moon::showAchievementById("achievement.get" + to_string(gHudDisplay.stars) + "Stars");
    }

    void updateAllTriggers(){
        actionTriggers();
        deathTrigger();
        starsTrigger();
    }

}

namespace MoonInternal {

    void setupAchievementEngine(string status){
        if(status == "Update" && gHudDisplay.flags != HUD_DISPLAY_NONE){
            if(Cheats.EnableCheats) {
                Moon::showAchievement(AchievementList::CHEATER);
                cheatsGotEnabled = true;
            }

            MoonTriggers::updateAllTriggers();
        }
        if(status == "Init"){
            Moon::registerHookListener({.hookName = POST_HUD_DRAW, .callback = [](HookCall call){
                for (auto &aEntry : entries) {
                    if( !aEntry->dead ) {
                        int soundID = SOUND_GENERAL_COIN;

                        bool shouldClose = aEntry->launchTime >= aEntry->achievement->duration;

                        float titleWidth = MoonGetTextWidth(aEntry->achievement->title, 0.8, false);
                        float descWidth  = MoonGetTextWidth(aEntry->achievement->description, 0.8, false);

                        int achievementWidth = 31 + 15 + max(titleWidth, descWidth);

                        int aX = GetScreenWidth(false) / 2 - aEntry->width / 2;
                        int aY = GetScreenHeight() - aEntry->y;

                        MoonDrawRectangle(aX, aY, aEntry->width, aEntry->height, {0, 0, 0, 150}, false);
                        MoonDrawTexture(GFX_DIMENSIONS_FROM_LEFT_EDGE(aX), aY, min(31, aEntry->width - 1), 31, sys_strdup(aEntry->achievement->icon.c_str()));
                        if(!shouldClose && aEntry->width >= achievementWidth * 0.9){
                            MoonDrawText(aX + 32 + 5, aY + 2,  aEntry->achievement->title,       0.8, {255,255,255,255}, true, false);
                            MoonDrawText(aX + 32 + 5, aY + 16, aEntry->achievement->description, 0.8, {255,255,255,255}, true, false);
                        }

                    #ifndef GAME_DEBUG
                        bool shouldUpdate = gMenuMode == -1 && !gWarpTransition.isActive;
                    #else
                        bool shouldUpdate = !gWarpTransition.isActive;
                    #endif
                        if(shouldUpdate){
                            int expectedY = aEntry->height + 45;

                            if (aEntry->launchTime == 1)
                                play_sound(soundID, gGlobalSoundSource);

                            aEntry->dead = aEntry->launchTime >= aEntry->achievement->duration + 35 && floor(aEntry->width) <= 0;
                            float dT = (aEntry->launchTime + 1.0f) / ((float)aEntry->achievement->duration);
                            if(aEntry->width <= 32 && !shouldClose)
                                aEntry->y = MathUtil::Lerp(aEntry->y, !shouldClose ? expectedY : 0, !shouldClose ? 0.17f : 0.28f);

                            if(ceil(aEntry->y) >= ceil(expectedY) || shouldClose)
                                aEntry->width = MathUtil::Lerp(aEntry->width, !shouldClose ? achievementWidth : 0, !shouldClose ? 0.2f : 0.45f);

                            aEntry->launchTime++;
                        }
                        return;
                    }
                }
            }});
        }
    }
}

namespace Moon {
    void showAchievement(Achievement* achievement){
        if(cheatsGotEnabled) return;

        if(find_if(entries.begin(), entries.end(),  [&cae = achievement] (auto &m) -> bool { return cae->id == m->achievement->id; }) != entries.end()) return;
        cout << "Achievement got triggered: " << achievement->title << endl;
        entries.push_back(new AchievementEntry({ .launchTime = 0, .dead = false, .achievement = achievement, .entryID = entries.size() }));
    }

    void showAchievementById(string id){
        if(registeredAchievements.find(id) == registeredAchievements.end()) return;
        Moon::showAchievement(registeredAchievements[id]);
    }

    Achievement* getAchievementById(string id){
        if(registeredAchievements.find(id) == registeredAchievements.end()) return NULL;
        return registeredAchievements[id];
    }
}

extern "C" {
    void show_achievement(char* id){
        Moon::showAchievementById(string(id));
    }
}