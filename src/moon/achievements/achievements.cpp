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
    Achievement* TRIPLE_JUMP = MoonAchievements::bind(new Achievement("achievement.doATripleJump", "textures/segment2/segment2.05C00.rgba16", "Getting higher", "Do a triple jump", 0, 5, nullptr));
    Achievement* CHEATER     = MoonAchievements::bind(new Achievement("",                          "textures/segment2/segment2.05C00.rgba16", "What a loser!", "Turn on the cheats", 0, 5, nullptr));
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
                        if(aEntry->width >= achievementWidth / 2){
                            MoonDrawText(aX + 32 + 5, aY + 2,      achievement.first->title,       0.8, {255,255,255,255}, true, false);
                            MoonDrawText(aX + 32 + 5, aY + 16, achievement.first->description, 0.8, {255,255,255,255}, true, false);
                        }
                    }

                    aEntry->width = MathUtil::Lerp(aEntry->width, !shouldClose ? achievementWidth : 0, !shouldClose ? 0.1f : 0.35f);
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