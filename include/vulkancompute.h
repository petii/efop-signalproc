#pragma once

//https://github.com/Erkaman/vulkan_minimal_compute/blob/master/src/main.cpp

#include "vulkan/vulkan.h"
#include "vulkanframe.h"
#include "vulkanutilities.h"
#include <cstring>

struct VulkanCompute {
    static const uint32_t bufferSize = 128; //play around with this

    VkDevice computeDevice;
    uint32_t queueFamilyIndex;
    VkQueue computeQueue;

    VkBuffer inputBuffer;
    VkDeviceMemory inputBufferMemory;
    VkBuffer resultBuffer;
    VkDeviceMemory resultBufferMemory;

    VkShaderModule computeShader;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSetLayout> descriptorSetLayout;
    VkDescriptorSet descriptorSet;

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
        inputBuffer(utility::createBuffer(computeDevice, bufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)),
        resultBuffer(utility::createBuffer(computeDevice,bufferSize,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)),
        computeShader(utility::createShaderModule(computeDevice,"src/shaders/comp.spv")),
        commandPool(utility::createCommandPool(computeDevice,queueFamilyIndex))
    {
        utility::createDescriptorPool(computeDevice,descriptorPool);
        utility::createDescriptorSetLayout(computeDevice,descriptorSetLayout);
        utility::createDescriptorSet(
            computeDevice,
            descriptorPool,
            descriptorSetLayout,
            descriptorSet
        );
        utility::createPipelineLayout(
            computeDevice,
            computeShader,
            descriptorSetLayout,
            pipelineLayout
        );
        utility::createComputePipeline(
            computeDevice,
            computeShader,
            pipelineLayout,
            bufferSize,bufferSize,
            computePipeline
        );
        utility::allocateBufferMemory(
            vf.physicalDevice,
            computeDevice,
            inputBuffer,
            inputBufferMemory
        );
        utility::allocateBufferMemory(
            vf.physicalDevice,
            computeDevice,
            resultBuffer,
            resultBufferMemory
        );

        utility::allocateCommandBuffer(computeDevice, commandPool,commandBuffer);
        // Get a handle to the only member of the queue family.
        vkGetDeviceQueue(computeDevice, queueFamilyIndex, 0, &computeQueue);
        if (vkBindBufferMemory(
                computeDevice,inputBuffer, inputBufferMemory, 0 /*offset*/
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("Failed to bind memory to buffer!");
        }
        if (vkBindBufferMemory(
                computeDevice,resultBuffer, resultBufferMemory, 0 /*offset*/
            ) != VK_SUCCESS
        ) {
            throw std::runtime_error("Failed to bind memory to buffer!");
        }
        utility::bindBufferToDescriptor(computeDevice,inputBuffer,bufferSize,descriptorSet);
        utility::bindBufferToDescriptor(computeDevice,resultBuffer,bufferSize,descriptorSet);

        recordCommands();
    }

    ~VulkanCompute() {
        std::cout << "compute destructing\n";
        vkFreeMemory(computeDevice, inputBufferMemory,nullptr);
        vkFreeMemory(computeDevice, resultBufferMemory,nullptr);
        vkDestroyBuffer(computeDevice,inputBuffer,nullptr);
        vkDestroyBuffer(computeDevice,resultBuffer,nullptr);
        vkDestroyShaderModule(computeDevice, computeShader,nullptr);
        vkDestroyDescriptorSetLayout(computeDevice,descriptorSetLayout[0],nullptr);
        vkDestroyDescriptorSetLayout(computeDevice,descriptorSetLayout[1],nullptr);
        vkDestroyDescriptorPool(computeDevice,descriptorPool,nullptr);
        vkDestroyPipeline(computeDevice, computePipeline,nullptr);
        vkDestroyPipelineLayout(computeDevice,pipelineLayout,nullptr);
        vkDestroyCommandPool(computeDevice,commandPool,nullptr);
        vkDestroyDevice(computeDevice,nullptr);
    }



    void copyData(const std::vector<float>& data) {
        void* place = nullptr;
        vkMapMemory(
            computeDevice,
            inputBufferMemory,
            0,
            data.size(),
            0,
            &place
        );
        std::memcpy(place,data.data(),data.size()*sizeof(float));
        vkUnmapMemory(computeDevice,inputBufferMemory);
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
