#ifndef MoonAchievements
#define MoonAchievements

#ifdef __cplusplus
#include <string>
#include <vector>

class Achievement {
public:
    std::string id;
    std::string icon;
    std::string title;
    std::string description;
    Achievement* parent;
    int points;
    long long duration;
    Achievement(std::string id, std::string icon, std::string title, std::string description, int points, float duration, Achievement* parent){
        this->id = id;
        this->icon = icon;
        this->title = title;
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

    int state = 0;
    int width = 0;
    int height = 32;
    float x = 0;
    float y = 0;
};

extern std::vector<AchievementEntry*> entries;
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
