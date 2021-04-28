#include "mw-value.h"
#include <iostream>

#include "moon/ui/utils/moon-draw-utils.h"

// std::cout << ptr << std::endl;    
// int* a = static_cast<int*>(ptr);
// int  b = *a;

MWValue::MWValue(void* ptr, float x, float y, MWValueType type){
    this->value = ptr;
    this->x = x;
    this->y = y;
}

void MWValue::Init(){}
void MWValue::Draw(){
    MoonDrawText(this->x, this->y, this->focused ? "Focused" : this->selected ? "Selected" : "No selected", 1.0, {255, 255, 255, 255}, false);
}
void MWValue::Update(){}
void MWValue::Dispose(){}