#include "vulkanfourier.h"

#include <cstring>

#include "utility/pipelineutils.h"

void VulkanFourier::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (const auto &device : devices) {
    if (isDeviceSuitable(device)) {
      physicalDevice = device;
      return;
    }
  }
  // return VK_NULL_HANDLE;
  throw std::runtime_error("Failed to find a suitable GPU!");
}

void VulkanFourier::createInstance(const std::string &appName,
                                   std::vector<const char *> extensions) {
  if (enableValidationLayers &&
      !checkValidationLayerSupport(validationLayers)) {
    throw std::runtime_error("Validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = appName.c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  if (enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create instance!");
  }
}

void VulkanFourier::DestroyDebugReportCallbackEXT(
    VkInstance instance, VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugReportCallbackEXT");
  if (func != nullptr) {
    func(instance, callback, pAllocator);
  }
}

VkResult VulkanFourier::CreateDebugReportCallbackEXT(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugReportCallbackEXT *pCallback) {
  auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugReportCallbackEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pCallback);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void VulkanFourier::setupDebugCallback(VkInstance instance) {
  VkDebugReportCallbackCreateInfoEXT createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  createInfo.flags =
      VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
  createInfo.pfnCallback = debugCallback;

  if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug callback!");
  }
}

bool VulkanFourier::checkValidationLayerSupport(
    const std::vector<const char *> &validationLayers) {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (std::string(layerName) == std::string(layerProperties.layerName)) {
        layerFound = true;
        break;
      }
    }
    if (!layerFound) {
      return false;
    }
  }
  return true;
}

void VulkanFourier::addInput(const std::vector<double> &data) {
  for (int i = 0; i < data.size(); ++i) {
    fData[i] = static_cast<float>(data[i]);
  }
  // runHanning();
  // std::memcpy(
  //     pData,
  //     data.data(),
  //     data.size()
  // );
}

std::vector<double> VulkanFourier::getResult() const {
  void *pData;
  vkMapMemory(device, bufferMemory, bufferSize,
              // 0,
              bufferSize / 2, 0, &pData);
  std::vector<double> data(windowSize / 2);
  std::memcpy(data.data(), pData,
              bufferSize / 2 // * sizeof(float)
  );
  vkUnmapMemory(device, bufferMemory);
  return data;
}

void VulkanFourier::runHanning() {
  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &hanningCommandBuffer;

  vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

  vkQueueWaitIdle(queue);
}

void VulkanFourier::runTransform() {
  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

  // TODO:do something instead of waiting
  vkQueueWaitIdle(queue);
}

void VulkanFourier::createComputeLogicalDevice(
    VkPhysicalDevice physicalDevice,
    const std::vector<const char *> &enabledLayers) {
  VkDeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = getComputeQueueFamilyIndex(physicalDevice);
  queueCreateInfo.queueCount =
      1; // create one queue in this family. We don't need more.
  float queuePriorities =
      1.0; // we only have one queue, so this is not that imporant.
  queueCreateInfo.pQueuePriorities = &queuePriorities;

  VkDeviceCreateInfo deviceCreateInfo = {};

  // TODO: extend this if needed
  VkPhysicalDeviceFeatures deviceFeatures = {};

  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.enabledLayerCount =
      enabledLayers.size(); // need to specify validation layers here as well.
  deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
  deviceCreateInfo.pQueueCreateInfos =
      &queueCreateInfo; // when creating the logical device, we also specify
                        // what queues it has.
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

  // VkDevice device;
  if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create logical compute device!");
  }
}

uint32_t
VulkanFourier::getComputeQueueFamilyIndex(VkPhysicalDevice physicalDevice) {
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           nullptr);

  // Retrieve all queue families.
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           queueFamilies.data());

  // Now find a family that supports compute.
  uint32_t index = 0;
  for (const VkQueueFamilyProperties &properties : queueFamilies) {
    if (properties.queueCount > 0 &&
        (properties.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
      return index;
    }
    ++index;
  }
  throw std::runtime_error(
      "Could not find a queue family that supports compute operations");
}

void VulkanFourier::allocateBufferMemory(VkPhysicalDevice physicalDevice) {
  // VkMemoryRequirements memoryRequirements;
  // vkGetBufferMemoryRequirements(device,inputBuffer, &memoryRequirements);

  VkPhysicalDeviceMemoryProperties properties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &properties);

  uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;

  for (uint32_t k = 0; k < properties.memoryTypeCount; k++) {
    if ((VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT &
         properties.memoryTypes[k].propertyFlags) &&
        (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT &
         properties.memoryTypes[k].propertyFlags) &&
        (memorySize <
         properties.memoryHeaps[properties.memoryTypes[k].heapIndex].size)) {
      memoryTypeIndex = k;
      break;
    }
  }
  if (memoryTypeIndex == VK_MAX_MEMORY_TYPES) {
    throw std::runtime_error("No suitable memory type index found!");
  }

  VkMemoryAllocateInfo memoryAllocateInfo = {};
  memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memoryAllocateInfo.allocationSize = memorySize * sizeof(float);
  memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
  // memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(
  //     physicalDevice,
  //     memoryRequirements,
  //     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
  //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
  // );

  if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &bufferMemory) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate device memory!");
  }
}

void VulkanFourier::createBuffers() {
  // TODO: error handling!
  VkBufferCreateInfo inputBufferCreateInfo = {};
  inputBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  inputBufferCreateInfo.size = bufferSize; // buffer size in bytes.
  inputBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  inputBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  vkCreateBuffer(device, &inputBufferCreateInfo, nullptr, &inputBuffer);
  // TODO: make use of memory requirements at allocation
  // for now it should shut up the validator :)
  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(device, inputBuffer, &memoryRequirements);

  vkBindBufferMemory(device, inputBuffer, bufferMemory, 0);

  VkBufferCreateInfo resultBufferCreateInfo = {};
  resultBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  resultBufferCreateInfo.size = 2 * bufferSize; // buffer size in bytes.
  resultBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  resultBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  vkCreateBuffer(device, &resultBufferCreateInfo, nullptr, &resultBuffer);
  vkGetBufferMemoryRequirements(device, resultBuffer, &memoryRequirements);

  vkBindBufferMemory(device, resultBuffer, bufferMemory, bufferSize);
}

void VulkanFourier::createShaderModule(const std::string &file) {
  auto shaderCode = util::pipeline::readShader(file);
  VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.pCode =
      reinterpret_cast<uint32_t *>(shaderCode.data());
  shaderModuleCreateInfo.codeSize = shaderCode.size();

  if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module!");
  }
}

void VulkanFourier::createDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[2] = {
      {
          0,                                 // uint32_t              binding;
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // VkDescriptorType descriptorType;
          1,                           // uint32_t              descriptorCount;
          VK_SHADER_STAGE_COMPUTE_BIT, // VkShaderStageFlags    stageFlags;
          0 // const VkSampler*      pImmutableSamplers;
      },
      {
          1,                                 // uint32_t              binding;
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // VkDescriptorType descriptorType;
          1,                           // uint32_t              descriptorCount;
          VK_SHADER_STAGE_COMPUTE_BIT, // VkShaderStageFlags    stageFlags;
          0 // const VkSampler*      pImmutableSamplers;
      }};
  VkDescriptorSetLayoutCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.bindingCount = 2;
  createInfo.pBindings = descriptorSetLayoutBinding;

  if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout!");
  }
}

void VulkanFourier::createDescriptorPool() {
  VkDescriptorPoolSize descriptorPoolSize = {};
  descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorPoolSize.descriptorCount = 2;

  VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
  descriptorPoolCreateInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolCreateInfo.maxSets =
      1; // we only need to allocate one descriptor set from the pool.
  descriptorPoolCreateInfo.poolSizeCount = 1;
  descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

  vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr,
                         &descriptorPool);
}

void VulkanFourier::allocateDescriptorSet() {
  VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
  descriptorSetAllocateInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descriptorSetAllocateInfo.descriptorPool =
      descriptorPool; // pool to allocate from.
  descriptorSetAllocateInfo.descriptorSetCount =
      1; // allocate a single descriptor set.
  descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

  vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet);
}

void VulkanFourier::connectBuffersToDescriptorSets() {
  VkDescriptorBufferInfo inDBI = {};
  inDBI.buffer = inputBuffer;
  inDBI.offset = 0;
  inDBI.range = bufferSize;

  VkDescriptorBufferInfo resDBI = {};
  resDBI.buffer = resultBuffer;
  resDBI.offset = 0;
  resDBI.range = VK_WHOLE_SIZE;

  VkWriteDescriptorSet writeDescriptorSet[2] = {
      {
          VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, // VkStructureType sType;
          nullptr,       // const void*                      pNext;
          descriptorSet, // VkDescriptorSet                  dstSet;
          0,             // uint32_t                         dstBinding;
          0,             // uint32_t                         dstArrayElement;
          1,             // uint32_t                         descriptorCount;
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // VkDescriptorType descriptorType;
          nullptr, // const VkDescriptorImageInfo*     pImageInfo;
          &inDBI,  // const VkDescriptorBufferInfo*    pBufferInfo;
          nullptr  // const VkBufferView*              pTexelBufferView;
      },
      {
          VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, // VkStructureType sType;
          nullptr,       // const void*                      pNext;
          descriptorSet, // VkDescriptorSet                  dstSet;
          1,             // uint32_t                         dstBinding;
          0,             // uint32_t                         dstArrayElement;
          1,             // uint32_t                         descriptorCount;
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // VkDescriptorType descriptorType;
          nullptr, // const VkDescriptorImageInfo*     pImageInfo;
          &resDBI, // const VkDescriptorBufferInfo*    pBufferInfo;
          nullptr  // const VkBufferView*              pTexelBufferView;
      }};
  vkUpdateDescriptorSets(device, 2, writeDescriptorSet, 0, nullptr);
}

void VulkanFourier::createComputePipelineLayout() {
  VkPipelineLayoutCreateInfo pllci = {};
  pllci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pllci.setLayoutCount = 1;
  pllci.pSetLayouts = &descriptorSetLayout;
  vkCreatePipelineLayout(device, &pllci, nullptr, &pipelineLayout);
}

void VulkanFourier::createComputePipelines() {
  // Set compute buffer size as a specialization constant (in shader)
  uint32_t specData[] = {windowSize};

  VkSpecializationMapEntry specMapEntry = {};
  specMapEntry.constantID = 21;
  specMapEntry.offset = 0;
  specMapEntry.size = sizeof(uint32_t);

  VkSpecializationInfo specInfo = {};
  specInfo.mapEntryCount = 1;
  specInfo.pMapEntries = &specMapEntry;
  specInfo.dataSize = sizeof(uint32_t);
  specInfo.pData = specData;

  VkPipelineShaderStageCreateInfo ssci = {};
  ssci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  ssci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  ssci.module = shaderModule;
  ssci.pName = "main";
  ssci.pSpecializationInfo = &specInfo;

  VkComputePipelineCreateInfo cplci = {};
  cplci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  cplci.stage = ssci;
  cplci.layout = pipelineLayout;

  vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &cplci, nullptr,
                           &pipeline);

  auto shaderCode = util::pipeline::readShader("src/shaders/hann.spv");
  auto hanningShader = util::pipeline::createShaderModule(device,shaderCode);

  VkPipelineShaderStageCreateInfo hsi = {};
  hsi.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  hsi.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  hsi.module = hanningShader;
  hsi.pName = "main";
  hsi.pSpecializationInfo = &specInfo;

  cplci.stage = hsi;

  vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &cplci, nullptr,
                           &hanningPipeline);

  vkDestroyShaderModule(device,hanningShader,nullptr);
}

void VulkanFourier::createCommandPool() {
  VkCommandPoolCreateInfo commandPoolCreateInfo = {};
  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.flags = 0;
  commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

  vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);
}

void VulkanFourier::allocateCommandBuffers() {
  VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
  commandBufferAllocateInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.commandPool =
      commandPool; // specify the command pool to allocate from.
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount =
      1; // allocate a single command buffer.
  vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);

  vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
                           &hanningCommandBuffer);
}

void VulkanFourier::recordCommands() {
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
  vkCmdDispatch(commandBuffer,
                // windowSize,// uint32_t groupCountX,
                workGroupCount, // uint32_t groupCountX,
                1,              // uint32_t groupCountY,
                1               // uint32_t groupCountZ
  );
  vkEndCommandBuffer(commandBuffer);

  vkBeginCommandBuffer(hanningCommandBuffer, &beginInfo);

  vkCmdBindPipeline(hanningCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    hanningPipeline);
  vkCmdBindDescriptorSets(hanningCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
  vkCmdDispatch(hanningCommandBuffer,
                // windowSize,// uint32_t groupCountX,
                workGroupCount, // uint32_t groupCountX,
                1,              // uint32_t groupCountY,
                1               // uint32_t groupCountZ
  );

  vkEndCommandBuffer(hanningCommandBuffer);
}
