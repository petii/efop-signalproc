#include "vulkangraphics.h"
#include <set>
#include "utility/swapchainutils.h"

VulkanGraphics::QueueFamilyIndices::QueueFamilyIndices(
        VkPhysicalDevice device,
        const VkSurfaceKHR& surface
) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, nullptr
    );
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, queueFamilies.data()
    );
    int index = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 &&
            queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT
        ) {
            graphicsFamily = index;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device,index,surface,&presentSupport
        );
        if (queueFamily.queueCount > 0 && presentSupport) {
            presentFamily = index;
        }
        if (isComplete()) return;
        ++index;
    }
}

void VulkanGraphics::createGraphicsLogicalDevice(
    const VkPhysicalDevice& physicalDevice,
    const std::vector<const char*>& deviceExtensions
) {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = { qfi.graphicsFamily, qfi.presentFamily };

    float queuePriority = 1.0f;
    for (int queueFamilyIndex : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo qCreateInfo = {};
        qCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qCreateInfo.queueFamilyIndex = queueFamilyIndex;
        qCreateInfo.queueCount = 1;
        qCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(qCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo dCreateInfo = {};
    dCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    dCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    dCreateInfo.pEnabledFeatures = &deviceFeatures;
    dCreateInfo.enabledExtensionCount = deviceExtensions.size();
    dCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(
        physicalDevice,
        &dCreateInfo,
        nullptr,
        &device
    ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device for graphics!");
    }
}

void VulkanGraphics::createSwapChain(const VulkanFrame& vf) {
    util::swapchain::SupportDetails swapChainSupport =
        util::swapchain::querySupportDetails(vf.physicalDevice, vf.surface);
    VkSurfaceFormatKHR surfaceFormat =
        util::swapchain::chooseSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode =
        util::swapchain::choosePresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = vf.chooseSwapExtent(swapChainSupport.capabilities);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount
    ) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vf.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] =
        {(uint32_t) qfi.graphicsFamily, (uint32_t) qfi.presentFamily};

    if (qfi.graphicsFamily != qfi.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void VulkanGraphics::createImageViews() {}
void VulkanGraphics::createRenderPass() {}
void VulkanGraphics::createDescriptorSetLayout() {}
void VulkanGraphics::createGraphicsPipeline() {}
void VulkanGraphics::createFramebuffers() {}
void VulkanGraphics::createCommanPool() {}
void VulkanGraphics::createVertexBuffer() {}
void VulkanGraphics::createIndexBuffer() {}
void VulkanGraphics::createUniformBuffer() {}
void VulkanGraphics::createDescriptorPool() {}
void VulkanGraphics::createDescriptorSet() {}
void VulkanGraphics::createCommandBuffers() {}
void VulkanGraphics::createSemaphores() {}
