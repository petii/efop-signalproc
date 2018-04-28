#pragma once

//https://github.com/Erkaman/vulkan_minimal_compute/blob/master/src/main.cpp

#include "vulkan/vulkan.h"
// #include "vulkan/vulkan.hpp"
#include "vulkanframe.h"
#include "vulkanutilities.h"

struct VulkanCompute {
    static const uint32_t bufferSize = 1024; //play around with this

    VkDevice computeDevice;
    uint32_t queueFamilyIndex;
    VkQueue computeQueue;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;

    VkShaderModule computeShader;
    VkPipelineLayout pipelineLayout;
    VkPipeline computePipeline;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VulkanCompute(const VulkanFrame& vf):
            computeDevice(utility::createComputeLogicalDevice(
                vf.physicalDevice,
                vf.validationLayers
            )),
            queueFamilyIndex(utility::getComputeQueueFamilyIndex(vf.physicalDevice)),
            buffer(utility::createBuffer(computeDevice, bufferSize)),
            bufferMemory(utility::allocateBufferMemory(
                vf.physicalDevice,
                computeDevice,
                buffer
            )),
            descriptorPool(utility::createDescriptorPool(computeDevice)),
            descriptorSetLayout(utility::createDescriptorSetLayout(computeDevice)),
            descriptorSet(utility::createDescriptorSet(
                computeDevice,
                descriptorPool,
                descriptorSetLayout)
            ),
            computeShader(utility::createShaderModule(computeDevice,"src/shaders/comp.spv")),
            pipelineLayout(utility::createPipelineLayout(computeDevice,computeShader,descriptorSetLayout)),
            computePipeline(utility::createComputePipeline(
                computeDevice,
                computeShader,
                pipelineLayout
            )),
            commandPool(utility::createCommandPool(computeDevice,queueFamilyIndex)),
            commandBuffer(utility::allocateCommandBuffer(computeDevice, commandPool))
    {
        // Get a handle to the only member of the queue family.
        vkGetDeviceQueue(computeDevice, queueFamilyIndex, 0, &computeQueue);
        if (vkBindBufferMemory(
                computeDevice,buffer, bufferMemory, 0 /*offset*/
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("Failed to bind memory to buffer!");
        }
        utility::bindBufferToDescriptor(computeDevice,buffer,bufferSize,descriptorSet);

        recordCommands();
    }

    ~VulkanCompute() {
        std::cout << "compute destructing\n";
        vkFreeMemory(computeDevice, bufferMemory,nullptr);
        vkDestroyBuffer(computeDevice,buffer,nullptr);
        vkDestroyShaderModule(computeDevice, computeShader,nullptr);
        vkDestroyDescriptorSetLayout(computeDevice,descriptorSetLayout,nullptr);
        vkDestroyDescriptorPool(computeDevice,descriptorPool,nullptr);
        vkDestroyPipeline(computeDevice, computePipeline,nullptr);
        vkDestroyPipelineLayout(computeDevice,pipelineLayout,nullptr);
        vkDestroyCommandPool(computeDevice,commandPool,nullptr);
        vkDestroyDevice(computeDevice,nullptr);
    }

    void runCommandBuffer() {
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkFence fence;
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        vkCreateFence(computeDevice,&fenceCreateInfo, nullptr, &fence);

        vkQueueSubmit(computeQueue, 1, &submitInfo, fence);

        vkWaitForFences(computeDevice, 1, &fence, VK_TRUE, 1000000000); //timeout in nanoseconds
        vkDestroyFence(computeDevice, fence, nullptr);
    }

    void recordCommands() {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        if (vkBeginCommandBuffer(commandBuffer,&beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording to command buffer!");
        }
        //TODO: error handling
        vkCmdBindPipeline(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            computePipeline
        );
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            pipelineLayout,
            0,1, //firstset, descriptorsetcount
            &descriptorSet,
            0,nullptr //dynamic offset
        );
        vkCmdDispatch(commandBuffer, bufferSize,1,1);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to end recording to command buffer!");
        }
    }
};
