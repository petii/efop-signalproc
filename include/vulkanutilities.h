#pragma once

#include "vulkan/vulkan.h"
// #include "vulkan/vulkan.hpp"

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
    return VK_NULL_HANDLE;
}

uint32_t getComputeQueueFamilyIndex(VkPhysicalDevice physicalDevice) {
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyCount,
        nullptr
    );

    // Retrieve all queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyCount,
        queueFamilies.data()
    );

    // Now find a family that supports compute.
    uint32_t index = 0;
    for (const VkQueueFamilyProperties& properties : queueFamilies ) {
        if (properties.queueCount > 0 &&
            (properties.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            return index;
        }
        ++index;
    }
    throw std::runtime_error(
        "Could not find a queue family that supports compute operations"
    );
}

VkDevice createComputeLogicalDevice(
        VkPhysicalDevice physicalDevice,
        const std::vector<const char*>& enabledLayers
) {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = getComputeQueueFamilyIndex(physicalDevice);
    queueCreateInfo.queueCount = 1; // create one queue in this family. We don't need more.
    float queuePriorities = 1.0;  // we only have one queue, so this is not that imporant.
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    VkDeviceCreateInfo deviceCreateInfo = {};

    //TODO: extend this if needed
    VkPhysicalDeviceFeatures deviceFeatures = {};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.enabledLayerCount = enabledLayers.size();  // need to specify validation layers here as well.
    deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; // when creating the logical device, we also specify what queues it has.
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VkDevice device;
    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical compute device!");
    }
    return device;
}

VkBuffer createBuffer(VkDevice device,VkDeviceSize size) {
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    //TODO: figure out buffer usage
    bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer buffer;
    if (vkCreateBuffer(device,&bufferCreateInfo,nullptr,&buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }
    return buffer;
}

uint32_t findMemoryTypeIndex(
    const VkPhysicalDevice& physicalDevice,
    const VkMemoryRequirements& memoryRequirements,
    const VkMemoryPropertyFlags& properties
) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    uint32_t index = 0;
    for (const auto& type : memoryProperties.memoryTypes) {
        if (
            (memoryRequirements.memoryTypeBits & (1 << index)) &&
            ((type.propertyFlags & properties) == properties )
        ) {
            return index;
        }
        ++index;
    }
    throw std::runtime_error("Suitable memory type not found!");
    // return -1;
}

VkDeviceMemory allocateBufferMemory(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkBuffer buffer
) {
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device,buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(
        physicalDevice,
        memoryRequirements,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );

    VkDeviceMemory deviceMemory;
    if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory)
            != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate device memory!");
    }
    return deviceMemory;
}


}
