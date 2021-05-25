#include "achievements.h"
#include "moon/mod-engine/hooks/hook.h"
#include "moon/ui/utils/moon-draw-utils.h"
#include "moon/ui/interfaces/moon-screen.h"

#include <vector>

std::map<Achievement*, bool> entries;

namespace AchievementList {
    Achievement* TRIPLE_JUMP = new Achievement("achievement.doATripleJump", "test", "Getting higher", "Do a triple jump", 0, nullptr);
};

namespace Moon {
    void showAchievement(Achievement* achievement){
        if(entries.find(achievement) == entries.end())
            entries[achievement] = false;
    }

    void showAchievementById(Achievement* achievement){

    }
}
int w = 0;
int h = 32;
int aId = -1;

bool launched = false;
int delay = 35;
int cgid = 0;

namespace MoonInternal{
    void setupAchievementEngine(std::string status){
        entries[AchievementList::TRIPLE_JUMP] = false;

        if(status == "Init"){

            Moon::registerHookListener({.hookName = HUD_DRAW, .callback = [](HookCall call){
                cgid++;
                int id = 0;
                if(cgid > 90){
                    launched = true;
                }

                if(!launched) return;

                for(auto &achievement : entries){
                    if(!achievement.second){

                        if(!(gGlobalTimer % delay)){
                            aId++;
                        }

                        switch(aId){
                            case 0:
                                if(w < 32){
                                    w += 4;
                                }
                                break;
                            case 1:
                                if(w < 128){
                                    w += 16;
                                }
                                delay = 60;
                                break;
                            case 2:
                                if(w > 0){
                                    w -= 36;
                                }
                                break;
                        }

                        MoonDrawRectangle(GetScreenWidth(false) / 2 - w / 2, GetScreenHeight() - h - 20, w, h, {255, 255, 255, 100}, false);

                        if(w >= 32)
                            MoonDrawRectangle(GetScreenWidth(false) / 2 - w / 2, GetScreenHeight() - h - 20, 32, 32, {0, 255, 200, 100}, false);
                        id++;
                    }
                }
            }});
        }
    }
}