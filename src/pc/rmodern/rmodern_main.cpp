#include "rmodern.h"

#include "render/rm_vk.h"
#include "window/rm_wapi_sdl.h"

#include <iostream>

void rmodern_init()
{
    rm_rapi* rapi = new rm_rapi_vk;
    rm_wapi* wapi = new rm_wapi_sdl;

    wapi->init();

    rapi->setWAPI(wapi);
    if(rapi->checkSupport())
        std::cout << "Vulkan is supported!!" << std::endl;
    else
        std::cout << "Vulkan is not supported :(" << std::endl;

    wapi->cleanup();
}