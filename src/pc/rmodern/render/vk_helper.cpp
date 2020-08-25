#include "vk_helper.h"

#include <mutex>

using std::mutex;

// Globals for use with GLAD
VkInstance gInstance = VK_NULL_HANDLE;
PFN_vkGetInstanceProcAddr gLoadFunc = nullptr;
mutex gLoadMutex;

GLADapiproc procLoadFunc(const char *name)
{
    if(gLoadFunc == nullptr)
        return nullptr;

    return (GLADapiproc) gLoadFunc(gInstance, name);
}

void loadVulkan(VkInstance instance, VkPhysicalDevice physicalDevice, PFN_vkGetInstanceProcAddr loadFunc)
{
    gLoadMutex.lock();

    gInstance = instance;
    gLoadFunc = loadFunc;

    gladLoadVulkan(physicalDevice, procLoadFunc);

    gInstance = nullptr;
    gLoadFunc = nullptr;

    gLoadMutex.unlock();
}