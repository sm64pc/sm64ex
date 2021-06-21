#ifndef MoonAchievements
#define MoonAchievements

#ifdef __cplusplus
#include <string>
#include <vector>
#include <map>

class Achievement {
public:
    std::string id;
    std::string icon;
    std::string lockedIcon;
    std::string title;
    std::string description;
    bool hasProgress = false;
    Achievement* parent;
    int sortId = 0;
    int points;
    long long duration;
    Achievement(std::string id, std::string icon, std::string title, std::string description, bool hasProgress, int points, float duration, Achievement* parent){
        this->id = id;
        this->icon = icon;
        this->lockedIcon = icon+".locked";
        this->title = title;
        this->hasProgress = hasProgress;
        this->description = description;
        this->duration = duration;
        this->parent = parent;
        this->points = points;
    }
};

struct AchievementEntry {
    long long launchTime;
    bool dead = false;
    Achievement* achievement;
    size_t entryID;

    int progress = 0;
    int state = 0;
    int width = 32;
    int height = 32;
    float x = 0;
    float y = 0;
};

extern std::map<int, std::vector<AchievementEntry*>> entries;
extern std::map<std::string, Achievement*> registeredAchievements;
extern bool cheatsGotEnabled;

namespace Moon {
    void showAchievement(Achievement* achievement);
    void showAchievementById(std::string id);

    Achievement* getAchievementById(std::string id);
}

namespace MoonInternal{
    void setupAchievementEngine(std::string status);
}

namespace MoonAchievements {
    Achievement* bind(Achievement* achievement);
}

namespace AchievementList {
    extern Achievement* TRIPLE_JUMP;
}

#else
void show_achievement(char* id);
#endif
#endif
