#pragma once

#include "fourierhandler.h"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class VulkanFourier : public FourierHandler {
#ifdef NDEBUG
  static const bool enableValidationLayers = false;
#else
  static const bool enableValidationLayers = true;
#endif

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_LUNARG_standard_validation"};

  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkInstance instance;
  VkPhysicalDevice physicalDevice;

  // this is only used in debug
  VkDebugReportCallbackEXT callback;

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

public:
  VulkanFourier() {
    createInstance(APPNAME, {});
    pickPhysicalDevice();
    if (enableValidationLayers) {
      setupDebugCallback(instance);
    }
    createComputeLogicalDevice(physicalDevice, validationLayers);

    // TODO: error handling!
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

    allocateBufferMemory(physicalDevice);
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

  ~VulkanFourier() {
    vkFreeMemory(device, bufferMemory, nullptr);
    vkDestroyBuffer(device, inputBuffer, nullptr);
    vkDestroyBuffer(device, resultBuffer, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyShaderModule(device, shaderModule, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
      DestroyDebugReportCallbackEXT(instance, callback, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
  }

private:
  void pickPhysicalDevice();
  void createInstance(const std::string &appName,
                      std::vector<const char *> extensions);
  void DestroyDebugReportCallbackEXT(VkInstance instance,
                                     VkDebugReportCallbackEXT callback,
                                     const VkAllocationCallbacks *pAllocator);

  VkResult CreateDebugReportCallbackEXT(
      VkInstance instance,
      const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator,
      VkDebugReportCallbackEXT *pCallback);

  void setupDebugCallback(VkInstance instance);

  bool checkValidationLayerSupport(
      const std::vector<const char *> &validationLayers);

  // TODO: set up this function according to the apps needs
  bool isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    return true;
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugReportFlagsEXT falags, VkDebugReportObjectTypeEXT objType,
      uint64_t obj, size_t location, int32_t code, const char *layerPrefix,
      const char *msg, void *userData) {
    std::cerr << "validation layer: " << msg << std::endl;
    return VK_FALSE;
  }
public:
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