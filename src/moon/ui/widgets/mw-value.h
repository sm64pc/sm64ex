#ifndef MoonWidgetValue
#define MoonWidgetValue

#include "moon/ui/interfaces/moon-widget.h"
#include <string>

struct MWValueBind{
    float *fvar;
    bool  *bvar;
    int   *ivar;
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