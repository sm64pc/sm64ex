#ifndef IMoonCategory
#define IMoonCategory

#include "moon/ui/interfaces/moon-widget.h"
#include <vector>
#include <string>

using namespace std;

class MoonCategory {
public:
    MoonCategory(string categoryName){
        this->categoryName = categoryName;
        this->titleKey = true;
    };
    bool titleKey = false;
    string categoryName;
    std::vector<MoonWidget*> catOptions;
};

#endif