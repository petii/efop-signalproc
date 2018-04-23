#pragma once

#include <vector>
#include "vulkan/vulkan.h"

struct VulkanFrame {
    #ifdef NDEBUG
    const bool enableValidationLayers = false;
    #else
    const bool enableValidationLayers = true;
    #endif

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VulkanFrame() {

    }
    
    ~VulkanFrame(){

    }
};
