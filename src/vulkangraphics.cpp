
#include "vulkangraphics.h"

void VulkanGraphics::createSurface() {}

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

void VulkanGraphics::createGraphicsLogicalDevice() {
            
}

void VulkanGraphics::createSwapChain() {}
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
