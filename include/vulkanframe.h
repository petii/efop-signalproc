#pragma once

//TODO:Figure out how to get glfw extensions through windowhandler
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <limits>
#include "vulkan/vulkan.h"
#include "windowhandler.h"

struct VulkanFrame {
    //used to read in spir-v shaders
    static std::vector<char> readFile(const std::string& filename);
    //TODO: move to utils!

// private:
    #ifdef NDEBUG
    static const bool enableValidationLayers = false;
    #else
    static const bool enableValidationLayers = true;
    #endif

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;// = VK_NULL_HANDLE;
    // VkDevice device;

    // this is only used in debug
    VkDebugReportCallbackEXT callback;

    const WindowHandler * const pWh;
public:
    VulkanFrame(
        const std::string& appName,
        //std::vector<const char*> extensions
        const WindowHandler& wh
    ): pWh(&wh) {
        //std::cout << "vulkanframe constructing\n";
        createInstance(appName,wh.getGLFWExtensions());
        pickPhysicalDevice();
        if (enableValidationLayers) {
            setupDebugCallback(instance);
        }
        createSurface(wh);
    }

    ~VulkanFrame(){
        //std::cout << "vulkanframe destructing\n";
        if (enableValidationLayers) {
            DestroyDebugReportCallbackEXT(instance,callback,nullptr);
        }
        vkDestroySurfaceKHR(instance,surface,nullptr);
        vkDestroyInstance(instance,nullptr);
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetWindowSize(pWh->window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

private:
    void pickPhysicalDevice();
    void createInstance(
            const std::string& appName,
            std::vector<const char*> extensions
            //TODO: include version as parameter
    );
    void DestroyDebugReportCallbackEXT(
        VkInstance instance,
        VkDebugReportCallbackEXT callback,
        const VkAllocationCallbacks* pAllocator
    );

    VkResult CreateDebugReportCallbackEXT(
            VkInstance instance,
            const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugReportCallbackEXT* pCallback
    );

    void setupDebugCallback(VkInstance instance);

    bool checkValidationLayerSupport(
        const std::vector<const char*>& validationLayers
    );

    //TODO: set up this function according to the apps needs
    bool isDeviceSuitable(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device,&properties);
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device,&features);
        return true;
    }

    void createSurface(
        const WindowHandler& wh
    ) {
        if (glfwCreateWindowSurface(
                instance,wh.window,nullptr,&surface
        ) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugReportFlagsEXT falags,
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

};
