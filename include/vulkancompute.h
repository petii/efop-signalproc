#pragma once

//https://github.com/Erkaman/vulkan_minimal_compute/blob/master/src/main.cpp

#include "vulkan/vulkan.h"
// #include "vulkan/vulkan.hpp"
#include "vulkanframe.h"
#include "vulkanutilities.h"

struct VulkanCompute {
    VkDevice computeDevice;
    uint32_t queueFamilyIndex;
    VkQueue computeQueue;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;

    VkPipeline computePipeline;
    VkPipelineLayout pipelineLayout;
    VkShaderModule computeShader;

    VkCommandBuffer commandBuffer;

    VulkanCompute(const VulkanFrame& vf):
            computeDevice(utility::createComputeLogicalDevice(
                vf.physicalDevice,
                vf.validationLayers
            )),
            queueFamilyIndex(utility::getComputeQueueFamilyIndex(vf.physicalDevice)),
            buffer(utility::createBuffer(computeDevice, 1024/*buffer size*/)),
            bufferMemory(utility::allocateBufferMemory(
                vf.physicalDevice,
                computeDevice,
                buffer
            ))
    {
        // Get a handle to the only member of the queue family.
        vkGetDeviceQueue(computeDevice, queueFamilyIndex, 0, &computeQueue);
        if (vkBindBufferMemory(
                computeDevice,buffer, bufferMemory, 0 /*offset*/
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("Failed to bind memory to buffer!");
        }
        // createDescriptorSetLayout();
        // createDescriptorSet();
        // createComputePipeline();
        // createCommandBuffer();
    }
};
