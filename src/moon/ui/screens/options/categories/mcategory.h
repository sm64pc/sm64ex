#ifndef IMoonCategory
#define IMoonCategory

#include "moon/texts/moon-loader.h"
#include "moon/ui/interfaces/moon-widget.h"
#include <vector>
#include <string>

using namespace std;

class MoonCategory {
public:
    MoonCategory(wstring categoryName){
        this->categoryName = categoryName;
        this->titleKey = true;
    };
    MoonCategory(string categoryName){
        this->categoryName = wide(categoryName);
        this->titleKey = true;
    };
    bool titleKey = false;
    wstring categoryName;
    std::vector<MoonWidget*> catOptions;
};

#endif