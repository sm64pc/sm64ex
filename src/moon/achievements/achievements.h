#ifndef MoonAchievements
#define MoonAchievements

#include <string>

class Achievement {
protected:
    std::string id;
    std::string icon;
    std::string title;
    std::string description;
    Achievement* parent;
    int points;
public:
    Achievement(std::string id, std::string icon, std::string title, std::string description, int points, Achievement* parent){
        this->id = id;
        this->icon = icon;
        this->title = title;
        this->description = description;
        this->parent = parent;
        this->points = points;
    }
};

namespace Moon {
    void showAchievement(Achievement* achievement);
}

namespace MoonInternal{
    void setupAchievementEngine(std::string status);
}

namespace AchievementList {
    extern Achievement* TRIPLE_JUMP;
}

#endif
