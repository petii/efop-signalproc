#pragma once

//https://github.com/Erkaman/vulkan_minimal_compute/blob/master/src/main.cpp

#include "vulkan/vulkan.h"
#include "vulkanframe.h"
#include "vulkanutilities.h"

struct VulkanCompute {
    VkDevice computeDevice;
    uint32_t queueFamilyIndex;
    VkQueue computeQueue;

    VulkanCompute(const VulkanFrame& vf):
            computeDevice(utility::createComputeLogicalDevice(
                vf.physicalDevice,
                vf.validationLayers
            )),
            queueFamilyIndex(utility::getComputeQueueFamilyIndex(vf.physicalDevice))
    {
        // Get a handle to the only member of the queue family.
        vkGetDeviceQueue(computeDevice, queueFamilyIndex, 0, &computeQueue);
        // createDevice();
        // createBuffer();
        // createDescriptorSetLayout();
        // createDescriptorSet();
        // createComputePipeline();
        // createCommandBuffer();
    }
};
