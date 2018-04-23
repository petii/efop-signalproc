#pragma once

#include "vulkan/vulkan.h"
#include "vulkanframe.h"
#include "vulkanutilities.h"

struct VulkanCompute {
    VkDevice computeDevice;

    VulkanCompute(const VulkanFrame& vf):
            computeDevice(utility::createComputeLogicalDevice(vf.physicalDevice))
    {
        // createDevice();
        // createBuffer();
        // createDescriptorSetLayout();
        // createDescriptorSet();
        // createComputePipeline();
        // createCommandBuffer();
    }
};
