#ifndef MoonAchievements
#define MoonAchievements

#ifdef __cplusplus
#include <string>

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
        this->duration = (long long)(duration * 1000LL);
        this->parent = parent;
        this->points = points;
    }
};

namespace Moon {
    void showAchievement(Achievement* achievement);
    void showAchievementById(std::string id);
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
