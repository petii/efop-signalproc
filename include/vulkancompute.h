#pragma once

//https://github.com/Erkaman/vulkan_minimal_compute/blob/master/src/main.cpp

#include "vulkan/vulkan.h"
#include "vulkanframe.h"
#include <cstring>

struct Complex {
    float real;
    float imag;
};

struct VulkanCompute {
    uint32_t windowSize; 
    uint32_t bufferSize;
    VkDeviceSize memorySize;

    VkDevice device;
    uint32_t queueFamilyIndex;

    VkBuffer inputBuffer;
    VkBuffer resultBuffer;
    VkDeviceMemory bufferMemory;

    VkShaderModule shaderModule;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkQueue queue;

    const int workGroupCount = 512;

    VulkanCompute(const VulkanFrame& vf, int windowSpan):
        windowSize(windowSpan),
        bufferSize(windowSize*sizeof(float)),
        memorySize(3*windowSize),
        queueFamilyIndex(getComputeQueueFamilyIndex(vf.physicalDevice))
        //,result(windowSize)
    {
        createComputeLogicalDevice(
            vf.physicalDevice,
            vf.validationLayers
        );
        //TODO: error handling!
        vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

        allocateBufferMemory(vf.physicalDevice);
        createBuffers();
        createShaderModule("src/shaders/comp.spv");
        createDescriptorSetLayout();
        createDescriptorPool();
        allocateDescriptorSet();
        connectBuffersToDescriptorSets();
        createComputePipelineLayout();
        createComputePipeline();
        createCommandPool();
        allocateCommandBuffer();
        recordCommands();
    }
    ~VulkanCompute();

    void copyDataToGPU(const std::vector<float>& data);
    std::vector<float> readDataFromGPU();

    void runCommandBuffer();
private:
    //std::vector<Complex> result;

    void createComputeLogicalDevice(
            VkPhysicalDevice physicalDevice,
            const std::vector<const char*>& enabledLayers
    );
    uint32_t getComputeQueueFamilyIndex(VkPhysicalDevice physicalDevice);
    void allocateBufferMemory(VkPhysicalDevice physicalDevice);
    void createBuffers();
    void createShaderModule(const std::string& file);
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void allocateDescriptorSet();
    void connectBuffersToDescriptorSets();
    void createComputePipelineLayout();
    void createComputePipeline();
    void createCommandPool();
    void allocateCommandBuffer();
    void recordCommands();
};
