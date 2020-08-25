#ifndef RM_WAPI_SDL_H
#define RM_WAPI_SDL_H

#include "rm_wapi.h"

class rm_wapi_sdl : public rm_wapi
{
public:
    virtual bool checkSupport();
    virtual void init();
    virtual void createWindow();
    virtual void destroyWindow();
    virtual void cleanup();
    
    virtual PFN_vkGetInstanceProcAddr getVulkanLoader();
};

#endif