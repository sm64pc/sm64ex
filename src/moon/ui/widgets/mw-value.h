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

    int   *index;
    std::vector<std::string>* values;
};

class MWValue : public MoonWidget {
    private:
        MWValueBind bind;
        std::string title;
    public:
        MWValue(MWValueBind bind, std::string title, float x, float y);
        void Init();
        void Draw();
        void Update();
        void Dispose();
};

#endif