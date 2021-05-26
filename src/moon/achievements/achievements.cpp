#include "achievements.h"
#include "moon/mod-engine/hooks/hook.h"
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/interfaces/moon-screen.h"
#include "moon/utils/umath.h"
#include <iostream>
#include <vector>

extern "C" {
#include "pc/cheats.h"
#include "moon/utils/moon-gfx.h"
#include "pc/platform.h"
#include "game/level_update.h"
}

struct AchievementEntry {
    long long launchTime;
    int state = 0;
    int width = 0;
    int height = 32;
    bool dead = false;
};

std::map<std::string, Achievement*> registeredAchievements;
std::map<Achievement*, AchievementEntry*> entries;

namespace AchievementList {
    /* Star achievements */
    Achievement* GET_1_STAR         = MoonAchievements::bind(new Achievement("achievement.get1Stars",   "textures/segment2/segment2.05C00.rgba16", "Your journey begins!", "Get one star",  0, 5, nullptr));
    Achievement* GET_30_STARS       = MoonAchievements::bind(new Achievement("achievement.get12Stars",  "textures/segment2/segment2.05C00.rgba16", "Cursed Power",         "Get 8 stars",   0, 5, GET_12_STARS));
    Achievement* GET_60_STARS       = MoonAchievements::bind(new Achievement("achievement.get50Stars",  "textures/segment2/segment2.05C00.rgba16", "Lucky Eight",          "Get 30 stars",  0, 5, GET_50_STARS));
    Achievement* GET_60_STARS       = MoonAchievements::bind(new Achievement("achievement.get50Stars",  "textures/segment2/segment2.05C00.rgba16", "Lucky Eight",          "Get 50 stars",  0, 5, GET_50_STARS));
    Achievement* GET_90_STARS       = MoonAchievements::bind(new Achievement("achievement.get70Stars",  "textures/segment2/segment2.05C00.rgba16", "Halfway done!",        "Get 70 stars",  0, 5, GET_70_STARS));
    Achievement* GET_120_STARS      = MoonAchievements::bind(new Achievement("achievement.get120Stars", "textures/segment2/segment2.05C00.rgba16", "The Completionist",    "Get 120 stars", 0, 5, GET_120_STARS));

    /* Cap Achievements */
    Achievement* UNLOCK_WING_CAP    = MoonAchievements::bind(new Achievement("achievement.unlockWingCap",   "textures/segment2/segment2.05C00.rgba16", "Super Man-rio",      "Unlock the wing cap",   0, 5, nullptr));
    Achievement* UNL0CK_METAL_CAP   = MoonAchievements::bind(new Achievement("achievement.unlockMetalCap",  "textures/segment2/segment2.05C00.rgba16", "Heavy-Headed",       "Unlock the metal cap",  0, 5, nullptr));
    Achievement* UNLOCK_VANISH_CAP  = MoonAchievements::bind(new Achievement("achievement.unlockVanishCap", "textures/segment2/segment2.05C00.rgba16", "Wait, Where Is He?", "Unlock the vanish cap", 0, 5, nullptr));

    /* Level Achievements */
    Achievement* GET_6_LEVEL_STARS  = MoonAchievements::bind(new Achievement("achievement.get6MainStars",         "textures/segment2/segment2.05C00.rgba16", "F Rank",  "Get all 6 Main Stars in One Level",      0, 5, nullptr));
    Achievement* GET_100_COIN_STAR  = MoonAchievements::bind(new Achievement("achievement.get100CoinStar",        "textures/segment2/segment2.05C00.rgba16", "E Rank",  "UGet a 100 Coin Star in One Level",      0, 5, nullptr));
    Achievement* GET_ALL_LVL_COINS  = MoonAchievements::bind(new Achievement("achievement.getAllCoins",           "textures/segment2/segment2.05C00.rgba16", "D Rank",  "Get all Coins in One Level",             0, 5, nullptr));
    Achievement* GET_FLOOR_0_STARS  = MoonAchievements::bind(new Achievement("achievement.getAllStarsInBasement", "textures/segment2/segment2.05C00.rgba16", "B Rank",  "Get all Main Stars in the Basement",     0, 5, nullptr));
    Achievement* GET_FLOOR_1_STARS  = MoonAchievements::bind(new Achievement("achievement.getAllStarsInFloor1",   "textures/segment2/segment2.05C00.rgba16", "C Rank",  "Get all Main Stars in the First Floor",  0, 5, nullptr));
    Achievement* GET_FLOOR_2_STARS  = MoonAchievements::bind(new Achievement("achievement.getAllStarsInFloor2",   "textures/segment2/segment2.05C00.rgba16", "A Rank",  "Get all Main Stars in the Second Floor", 0, 5, nullptr));
    Achievement* GET_FLOOR_3_STARS  = MoonAchievements::bind(new Achievement("achievement.getAllStarsInFloor3",   "textures/segment2/segment2.05C00.rgba16", "S Rank",  "Get all Main Stars in the Third Floor",  0, 5, nullptr));
    Achievement* GET_CASTLE_STARS   = MoonAchievements::bind(new Achievement("achievement.getAllCastleStars",     "textures/segment2/segment2.05C00.rgba16", "S+ Rank", "Get all Castle Secret Stars",            0, 5, nullptr));

    /* Boss Achievements */
    Achievement* DEFEAT_KINGBOB     = MoonAchievements::bind(new Achievement("achievement.beatKingBobOmb",    "textures/segment2/segment2.05C00.rgba16", "Explosive Test",               "Beat King Bob-Omb",                0, 5, nullptr));
    Achievement* DEFEAT_XINGWHOMP   = MoonAchievements::bind(new Achievement("achievement.beatKingWhomp",     "textures/segment2/segment2.05C00.rgba16", "Come On And Slam",             "Beat King Whomp",                  0, 5, nullptr));
    Achievement* DEFEAT_ALL_BOOS    = MoonAchievements::bind(new Achievement("achievement.beatAll3BoosOnLLL", "textures/segment2/segment2.05C00.rgba16", "Mario, Where Are You??",       "Beat all 3 Boos from BBH",         0, 5, nullptr));
    Achievement* DEFEAT_MRI         = MoonAchievements::bind(new Achievement("achievement.beatMr.I",          "textures/segment2/segment2.05C00.rgba16", "I vs Eye",                     "Beat Mr.I",                        0, 5, nullptr));
    Achievement* DEFEAT_ALL_BULLY   = MoonAchievements::bind(new Achievement("achievement.beatAllBigBullies", "textures/segment2/segment2.05C00.rgba16", "The Real Bully",               "Beat all big bullies from LLL",    0, 5, nullptr));
    Achievement* DEFEAT_EYEROK      = MoonAchievements::bind(new Achievement("achievement.beatEyerok",        "textures/segment2/segment2.05C00.rgba16", "Welcome To The Jam",           "Beat Eyerok",                      0, 5, nullptr));
    Achievement* DEFEAT_WIGGLER     = MoonAchievements::bind(new Achievement("achievement.beatWiggler",       "textures/segment2/segment2.05C00.rgba16", "Insecticude",                  "Beat Wiggler",                     0, 5, nullptr));
    Achievement* DEFEAT_BOWSER1     = MoonAchievements::bind(new Achievement("achievement.beatFirstBowser",   "textures/segment2/segment2.05C00.rgba16", "Bowser Trapped In The Dark",   "Beat first Bowser",                0, 5, nullptr));
    Achievement* DEFEAT_BOWSER2     = MoonAchievements::bind(new Achievement("achievement.beatSecondBowser",  "textures/segment2/segment2.05C00.rgba16", "Bowser Burned By The Lava",    "Beat second Bowser",               0, 5, nullptr));
    Achievement* DEFEAT_BOWSER3     = MoonAchievements::bind(new Achievement("achievement.beatFinalBowser",   "textures/segment2/segment2.05C00.rgba16", "Bowser Launched Into The Sky", "Beat final Bowser",                0, 5, nullptr));
    Achievement* DEFEAT_120S_BOWSER = MoonAchievements::bind(new Achievement("achievement.beatGame120Stars",  "textures/segment2/segment2.05C00.rgba16", "Power Battle",                 "Beat final Bowser with 120 stars", 0, 5, nullptr));

    /* Death Achievements */
    Achievement* DEATH_BY_ENEMY     = MoonAchievements::bind(new Achievement("achievement.deathByEnemy",      "textures/segment2/segment2.05C00.rgba16", "Classic Way",   "Get killed by a normal enemy", 0, 5, nullptr));
    Achievement* DEATH_BY_BOSS      = MoonAchievements::bind(new Achievement("achievement.deathByBoss",       "textures/segment2/segment2.05C00.rgba16", "Git Gud",       "Get killed by a boss",         0, 5, nullptr));
    Achievement* DEATH_BY_BOWSER    = MoonAchievements::bind(new Achievement("achievement.deathByBowser",     "textures/segment2/segment2.05C00.rgba16", "Bad Ending",    "Get killed by Bowser",         0, 5, nullptr));
    Achievement* DEATH_BY_CRUSHING  = MoonAchievements::bind(new Achievement("achievement.deathByCrushing",   "textures/segment2/segment2.05C00.rgba16", "Space Jam",     "Get crushed",                  0, 5, nullptr));
    Achievement* DEATH_BY_FALLING   = MoonAchievements::bind(new Achievement("achievement.deathByFalling",    "textures/segment2/segment2.05C00.rgba16", "My Leg!",       "Die by falling",               0, 5, nullptr));
    Achievement* DEATH_BY_FIRE      = MoonAchievements::bind(new Achievement("achievement.deathByFire",       "textures/segment2/segment2.05C00.rgba16", "Roasted Mario", "Die by fire",                  0, 5, nullptr));
    Achievement* DEATH_BY_SAND      = MoonAchievements::bind(new Achievement("achievement.deathBySand",       "textures/segment2/segment2.05C00.rgba16", "Sinked",        "Die by sinking sand",          0, 5, nullptr));
    Achievement* DEATH_BY_DROWNING  = MoonAchievements::bind(new Achievement("achievement.deathByDrowning",   "textures/segment2/segment2.05C00.rgba16", "Under The Sea", "Die by drowning",              0, 5, nullptr));

    /* Extra Achievements */
    Achievement* RELEASE_CHAIN_CHOMP = MoonAchievements::bind(new Achievement("achievement.releaseChainChomp", "textures/segment2/segment2.05C00.rgba16", "Git Gud",        "Get killed by a boss", 0, 5, nullptr));
    Achievement* JUMP_1000_TIMES     = MoonAchievements::bind(new Achievement("achievement.jump1000Times",     "textures/segment2/segment2.05C00.rgba16", "Bad Ending",     "Get killed by Bowser", 0, 5, nullptr));
    Achievement* TALK_25_TIMES       = MoonAchievements::bind(new Achievement("achievement.talk25Times",       "textures/segment2/segment2.05C00.rgba16", "Space Jam",      "Get crushed",          0, 5, nullptr));
    Achievement* SLIDE_20_TIMES      = MoonAchievements::bind(new Achievement("achievement.slide20Times",      "textures/segment2/segment2.05C00.rgba16", "My Leg!",        "Die by falling",       0, 5, nullptr));
    Achievement* WATCH_END_CREDITS   = MoonAchievements::bind(new Achievement("achievement.watchEndCredits",   "textures/segment2/segment2.05C00.rgba16", "Roasted Mario",  "Die by fire",          0, 5, nullptr));
    Achievement* TALK_WITH_YOSHI     = MoonAchievements::bind(new Achievement("achievement.talkWithYoshi",     "textures/segment2/segment2.05C00.rgba16", "Sinked",         "Die by sinking sand",  0, 5, nullptr));
    Achievement* TRIPLE_JUMP         = MoonAchievements::bind(new Achievement("achievement.doATripleJump",     "textures/segment2/segment2.05C00.rgba16", "Getting higher", "Do a triple jump",     0, 5, nullptr));
    Achievement* CHEATER             = MoonAchievements::bind(new Achievement("",                              "textures/segment2/segment2.05C00.rgba16", "What a loser!",  "You turned on cheats", 0, 5, nullptr));
};

namespace MoonAchievements {
    Achievement* bind(Achievement* achievement){
        registeredAchievements[achievement->id] = achievement;
        return achievement;
    }
}

bool cheatsGotEnabled = false;

namespace MoonInternal{

    void setupAchievementEngine(std::string status){
        if(status == "Update"){
            if(Cheats.EnableCheats) {
                Moon::showAchievement(AchievementList::CHEATER);
                cheatsGotEnabled = true;
            }
            Moon::showAchievementById("achievement.get" + std::to_string(gHudDisplay.stars) + "Stars");
        }
        if(status == "Init"){
            Moon::registerHookListener({.hookName = HUD_DRAW, .callback = [](HookCall call){
                int id = 0;

                for(auto &achievement : entries){
                    int achievementWidth = 128;
                    long long millis = moon_get_milliseconds();
                    AchievementEntry* aEntry = achievement.second;
                    if(aEntry->dead || achievement.second->launchTime >= millis) continue;
                    bool shouldClose = millis >= achievement.second->launchTime + achievement.first->duration;

                    int aX = GetScreenWidth(false) / 2 - aEntry->width / 2;
                    int aY = GetScreenHeight() - aEntry->height - 20;

                    MoonDrawRectangle(aX, aY, aEntry->width, aEntry->height, {0, 0, 0, 150}, false);
                    if(!shouldClose){
                        if(aEntry->width >= 32)
                            MoonDrawTexture(GFX_DIMENSIONS_FROM_LEFT_EDGE(aX), aY, 31, 31, sys_strdup(achievement.first->icon.c_str()));
                        if(aEntry->width >= achievementWidth * 0.9){
                            MoonDrawText(aX + 32 + 5, aY + 2,      achievement.first->title,       0.8, {255,255,255,255}, true, false);
                            MoonDrawText(aX + 32 + 5, aY + 16, achievement.first->description, 0.8, {255,255,255,255}, true, false);
                        }
                    }

                    aEntry->width = MathUtil::Lerp(aEntry->width, !shouldClose ? achievementWidth : 0, !shouldClose ? 0.2f : 0.35f);
                    aEntry->dead = shouldClose && aEntry->width <= 0;
                    break;
                    id++;
                }
            }});
        }
    }
}

namespace Moon {
    void showAchievement(Achievement* achievement){
        if(cheatsGotEnabled) return;
        if(entries.find(achievement) != entries.end()) return;
        long long time = 0;
        if(entries.size() > 0)
            for(auto &achievement : entries){
                if(achievement.second->dead) continue;
                long long now = moon_get_milliseconds();
                time += (now - achievement.second->launchTime) + achievement.first->duration;
            }
        entries[achievement] = new AchievementEntry({.launchTime = time == 0 ? moon_get_milliseconds() + 100 : time});
    }

    void showAchievementById(std::string id){
        if(registeredAchievements.find(id) == registeredAchievements.end()) return;
        Moon::showAchievement(registeredAchievements[id]);
    }
}

extern "C" {
    void show_achievement(char* id){
        Moon::showAchievementById(std::string(id));
    }
}