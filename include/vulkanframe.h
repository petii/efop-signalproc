#pragma once

//TODO:Figure out how to get glfw extensions through windowhandler
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include "vulkan/vulkan.h"
#include "vulkanutilities.h"

struct VulkanFrame {
// private:
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

    VkInstance instance;
    VkPhysicalDevice physicalDevice;// = VK_NULL_HANDLE;
    // VkDevice device;

    // this is only used in debug
    VkDebugReportCallbackEXT callback;
public:
    VulkanFrame(const std::string& appName,std::vector<const char*> extensions):
            // callback(setupDebugCallback()),
            instance (createInstance(appName,extensions)),
            physicalDevice(utility::pickPhysicalDevice(instance))
            //TODO: creating logical devices should be the subsystem's job
            // device(utility::createLogicalDevice(physicalDevice))
    {
        std::cout << "vulkanframe constructing\n";
        if (enableValidationLayers) {
            callback = utility::setupDebugCallback(instance);
        }
    }

    ~VulkanFrame(){
        std::cout << "vulkanframe destructing\n";
    }

private:
    VkInstance createInstance(
            const std::string& appName,
            std::vector<const char*> extensions
            //TODO: include version as parameter
    ) {
        if (enableValidationLayers &&
            !utility::checkValidationLayerSupport(validationLayers)
        ) {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }
        VkInstance instance;
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create instance!");
        }
        return instance;
    }
};
