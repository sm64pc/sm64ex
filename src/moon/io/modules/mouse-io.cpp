#include "mouse-io.h"

#ifndef WAPI_SDL2
#include <SDL2/SDL.h>
#endif

#ifdef WAPI_DXGI

#endif

unsigned int state;

void MouseIO::init(){
    this->xPos = 0;
    this->yPos = 0;
    this->xLocalPos = 0;
    this->yLocalPos = 0;
    this->xGlobalPos = 0;
    this->yGlobalPos = 0;
    this->vScroll = 0;
    this->hScroll = 0;
}

void MouseIO::update(){
#ifdef WAPI_SDL2
    SDL_ShowCursor(!this->hideCursor ? SDL_TRUE : SDL_FALSE);
    SDL_SetRelativeMouseMode(this->relativeMode ? SDL_TRUE : SDL_FALSE);
    SDL_GetRelativeMouseState(&this->xPos, &this->yPos);
    state = SDL_GetMouseState(&this->xLocalPos, &this->yLocalPos);
    SDL_GetGlobalMouseState(&this->xGlobalPos, &this->yGlobalPos);
#elif defined(WAPI_DXGI)
    //TODO: Implement directx hook
#endif
}

bool MouseIO::isBtnPressed(MouseBtn btn){
    return state & (int) btn;
}