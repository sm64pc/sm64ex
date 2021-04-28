#ifndef MoonWidgetValue
#define MoonWidgetValue

#include "moon/ui/interfaces/moon-widget.h"

enum MWValueType{
    INT, FLOAT, BOOL
};

class MWValue : public MoonWidget {
    private:
        void* value;
    public:
        MWValue(void* ptr, float x, float y, MWValueType type);
        void Init();
        void Draw();
        void Update();
        void Dispose();
};

#endif