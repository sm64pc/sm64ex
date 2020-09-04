#ifndef VK_HELPER_H
#define VK_HELPER_H

#include <glad/vulkan.h>

/**
 * @brief A thread-safe function that uses a particular VkInstance to load the Vulkan API using GLAD.
 * 
 * @param instance 
 * @param physicalDevice 
 * @param loadFunc 
 */
void loadVulkan(VkInstance instance, VkPhysicalDevice physicalDevice, PFN_vkGetInstanceProcAddr loadFunc);

#endif