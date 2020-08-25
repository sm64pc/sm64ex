#ifndef RM_WAPI_H
#define RM_WAPI_H

#include <glad/vulkan.h>

class rm_wapi
{
public:
    virtual bool checkSupport() = 0;
    virtual void init() = 0;
    virtual void createWindow() = 0;
    virtual void destroyWindow() = 0;
    virtual void cleanup() = 0;
    
    virtual PFN_vkGetInstanceProcAddr getVulkanLoader()
    {
        return nullptr;
    }
};

#endif