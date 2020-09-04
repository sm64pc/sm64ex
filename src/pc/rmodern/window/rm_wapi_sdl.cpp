#include "rm_wapi_sdl.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

// TODO: actually check for support
bool rm_wapi_sdl::checkSupport()
{
    return true;
}

void rm_wapi_sdl::init()
{
    SDL_Init(SDL_INIT_VIDEO);
}

void rm_wapi_sdl::createWindow()
{

}
 
void rm_wapi_sdl::destroyWindow()
{

}

void rm_wapi_sdl::cleanup()
{
    SDL_Quit();
}
    
PFN_vkGetInstanceProcAddr rm_wapi_sdl::getVulkanLoader()
{
    int result = SDL_Vulkan_LoadLibrary(NULL);

    if(result != 0)
        return nullptr;

    return (PFN_vkGetInstanceProcAddr) SDL_Vulkan_GetVkGetInstanceProcAddr();
}