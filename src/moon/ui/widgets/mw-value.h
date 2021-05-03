#ifndef MoonWidgetValue
#define MoonWidgetValue

#include "moon/ui/interfaces/moon-widget.h"
#include <string>
#include <vector>

struct MWValueBind{
    bool  *bvar;

    float *fvar;    
    int   *ivar;
    float max;
    float min;
    float step;
    void (*btn)();

    int   *index;
    std::vector<std::string>* values;
};

class MWValue : public MoonWidget {
    public:
        MWValueBind bind;
        std::string title;
        MWValue(float x, float y, std::string title, MWValueBind bind);
        void Init();
        void Draw();
        void Update();
        void Dispose();
};

#endif