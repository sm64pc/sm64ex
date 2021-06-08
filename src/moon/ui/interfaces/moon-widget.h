#ifndef MoonWidgetInterface
#define MoonWidgetInterface

class MoonScreen;
class MoonWidget {
public:
    float x;
    float y;
    float height = 16;
    bool enabled = true;
    bool centered = true;
    bool selectable = true;
    bool selected = false;
    bool focused = false;
    MoonScreen *parent;
    virtual void Init(){}
    virtual void Draw(){}
    virtual void Update(){}
    virtual void Dispose(){}
};

#endif