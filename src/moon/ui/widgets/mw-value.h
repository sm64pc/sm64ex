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
    std::vector<std::wstring>* values;
    bool valueKeys;

    uint32_t* bindKeys;
    std::string keyIcon;

    void (*callback)();
};

class MWValue : public MoonWidget {
    protected:
        bool titleKey;
    public:
        MWValueBind bind;
        std::wstring title;
        MWValue(float x, float y, std::wstring title, MWValueBind bind);
        MWValue(float x, float y, std::wstring title, MWValueBind bind, bool titleKey);
        MWValue(float x, float y, std::string title, MWValueBind bind);
        MWValue(float x, float y, std::string title, MWValueBind bind, bool titleKey);
        void Init();
        void Draw();
        void Update();
        void Dispose();
};

#endif