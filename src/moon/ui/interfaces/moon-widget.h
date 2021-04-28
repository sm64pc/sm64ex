#ifndef MoonWidgetInterface
#define MoonWidgetInterface

class MoonWidget {    
public:
    float x;
    float y;
    bool enabled = true;
    bool centered = true;
    bool selectable = true;
    bool selected = false;
    bool focused = false;
    virtual void Init(){}
    virtual void Draw(){}
    virtual void Update(){}
    virtual void Dispose(){}
};

#endif