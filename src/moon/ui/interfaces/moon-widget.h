#ifndef MoonWidgetInterface
#define MoonWidgetInterface

class MoonWidget {
public:
    virtual void Init(){}
    virtual void Draw(){}
    virtual void Update(){}
    virtual void Dispose(){}
};

#endif