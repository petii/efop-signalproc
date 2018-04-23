#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>


namespace utility{

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t obj,
        size_t location,
        int32_t code,
        const char* layerPrefix,
        const char* msg,
        void* userData
) {
    std::cerr << "validation layer: " << msg << std::endl;
    return VK_FALSE;
}
VkResult CreateDebugReportCallbackEXT(
        VkInstance instance,
        const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugReportCallbackEXT* pCallback
) {
    auto func =
        (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugReportCallbackEXT"
        );
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VkDebugReportCallbackEXT setupDebugCallback(VkInstance instance) {
    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags =
        VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = utility::debugCallback;

    VkDebugReportCallbackEXT callback;
    if (CreateDebugReportCallbackEXT(
                instance,
                &createInfo,
                nullptr,
                &callback
            ) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug callback!");
    }
    return callback;
}

bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (
                std::string(layerName)
                    ==
                std::string(layerProperties.layerName)
            ) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

//TODO: set up this function according to the apps needs
bool isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device,&properties);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device,&features);
    return true;
}

VkPhysicalDevice pickPhysicalDevice(VkInstance instance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (utility::isDeviceSuitable(device)) {
            // physicalDevice = device;
            // break;
            return device;
        }
    }
    // return VK_NULL_HANDLE;
    throw std::runtime_error("Failed to find a suitable GPU!");
}

VkDevice createGraphicsLogicalDevice(VkPhysicalDevice physicalDevice) {
    //TODO: this function
}

VkDevice createComputeLogicalDevice(VkPhysicalDevice physicalDevice) {

}


}
