#include "rm_vk.h"

#include <glad/vulkan.h>

#include "../window/rm_wapi.h"
#include "vk_helper.h"

void rm_rapi_vk::setWAPI(rm_wapi* wapi)
{
    this->mWAPI = wapi;
}

// TODO: add checks for physical device presence
bool rm_rapi_vk::checkSupport()
{
    if(this->mWAPI == nullptr)
        return false;

    this->mVkGetInstanceProcAddr = this->mWAPI->getVulkanLoader();

    if(!this->mVkGetInstanceProcAddr)
        return false;

    loadVulkan(VK_NULL_HANDLE, VK_NULL_HANDLE, this->mVkGetInstanceProcAddr);

    if(vkCreateInstance)
        return true;
    else
        return false;
}

bool rm_rapi_vk::init()
{
    return false;
}

rm_mesh* rm_rapi_vk::createMesh()
{
    return nullptr;
}