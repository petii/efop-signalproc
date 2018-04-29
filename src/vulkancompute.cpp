#include "vulkancompute.h"

VulkanCompute::~VulkanCompute() {
    vkFreeMemory(device,bufferMemory,nullptr);
    vkDestroyBuffer(device, inputBuffer,nullptr);
    vkDestroyBuffer(device, resultBuffer,nullptr);
    vkDestroyDescriptorPool(device,descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device,descriptorSetLayout, nullptr);
    vkDestroyShaderModule(device,shaderModule,nullptr);
    vkDestroyPipeline(device,pipeline,nullptr);
    vkDestroyPipelineLayout(device,pipelineLayout,nullptr);
    vkFreeCommandBuffers(device,commandPool,1,&commandBuffer);
    vkDestroyCommandPool(device,commandPool,nullptr);
    vkDestroyDevice(device,nullptr);
}

void VulkanCompute::copyDataToGPU(const std::vector<float>& data) {
    void *pData;
    vkMapMemory(
        device,
        bufferMemory,
        0,
        bufferSize,
        0,
        &pData
    );
    float* fData = (float*)pData;
    for (int i = 0; i<data.size(); ++i) {
        fData[i] = data[i];
    }
    // std::memcpy(
    //     pData,
    //     data.data(),
    //     data.size()
    // );
    vkUnmapMemory(device,bufferMemory);
}

std::vector<float> VulkanCompute::readDataFromGPU() {
void *pData;
    vkMapMemory(
        device,
        bufferMemory,
        bufferSize,
        // 0,
        bufferSize,
        0,
        &pData
    );
    std::vector<float> data(windowSize);
    std::memcpy(
        data.data(),
        pData,
        bufferSize// * sizeof(float)
    );
    vkUnmapMemory(device,bufferMemory);
    return data;
}

void VulkanCompute::runCommandBuffer() {
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue,1,&submitInfo,VK_NULL_HANDLE);

    //TODO:do something instead of waiting
    vkQueueWaitIdle(queue);
}

void VulkanCompute::createComputeLogicalDevice(
        VkPhysicalDevice physicalDevice,
        const std::vector<const char*>& enabledLayers
) {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = getComputeQueueFamilyIndex(physicalDevice);
    queueCreateInfo.queueCount = 1; // create one queue in this family. We don't need more.
    float queuePriorities = 1.0;  // we only have one queue, so this is not that imporant.
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    VkDeviceCreateInfo deviceCreateInfo = {};

    //TODO: extend this if needed
    VkPhysicalDeviceFeatures deviceFeatures = {};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.enabledLayerCount = enabledLayers.size();  // need to specify validation layers here as well.
    deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; // when creating the logical device, we also specify what queues it has.
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    // VkDevice device;
    if (vkCreateDevice(
            physicalDevice,
            &deviceCreateInfo,
            nullptr,
            &device
    ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical compute device!");
    }
}

uint32_t VulkanCompute::getComputeQueueFamilyIndex(VkPhysicalDevice physicalDevice) {
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyCount,
        nullptr
    );

    // Retrieve all queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyCount,
        queueFamilies.data()
    );

    // Now find a family that supports compute.
    uint32_t index = 0;
    for (const VkQueueFamilyProperties& properties : queueFamilies ) {
        if (properties.queueCount > 0 &&
            (properties.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            return index;
        }
        ++index;
    }
    throw std::runtime_error(
        "Could not find a queue family that supports compute operations"
    );
}

void VulkanCompute::allocateBufferMemory(
    VkPhysicalDevice physicalDevice
) {
    // VkMemoryRequirements memoryRequirements;
    // vkGetBufferMemoryRequirements(device,inputBuffer, &memoryRequirements);

    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &properties);

    uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;

    for (uint32_t k = 0; k < properties.memoryTypeCount; k++) {
      if ((VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & properties.memoryTypes[k].propertyFlags) &&
        (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & properties.memoryTypes[k].propertyFlags) &&
        (memorySize < properties.memoryHeaps[properties.memoryTypes[k].heapIndex].size)) {
        memoryTypeIndex = k;
        break;
      }
    }
    if (memoryTypeIndex == VK_MAX_MEMORY_TYPES){
        throw std::runtime_error("No suitable memory type index found!");
    }

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memorySize * sizeof(float) ;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    // memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(
    //     physicalDevice,
    //     memoryRequirements,
    //     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    // );

    if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &bufferMemory)
            != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate device memory!");
    }
}

void VulkanCompute::createBuffers() {
    //TODO: error handling!
    VkBufferCreateInfo inputBufferCreateInfo = {};
    inputBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    inputBufferCreateInfo.size = bufferSize; // buffer size in bytes.
    inputBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    inputBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(
        device,
        &inputBufferCreateInfo,
        nullptr,
        &inputBuffer
    );
    //TODO: make use of memory requirements at allocation
    //for now it should shut up the validator :)
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device,inputBuffer, &memoryRequirements);

    vkBindBufferMemory(
        device,
        inputBuffer,
        bufferMemory,
        0
    );

    VkBufferCreateInfo resultBufferCreateInfo = {};
    resultBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    resultBufferCreateInfo.size = 2*bufferSize; // buffer size in bytes.
    resultBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    resultBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(
        device,
        &resultBufferCreateInfo,
        nullptr,
        &resultBuffer
    );
    vkGetBufferMemoryRequirements(device,resultBuffer, &memoryRequirements);

    vkBindBufferMemory(
        device,
        resultBuffer,
        bufferMemory,
        bufferSize
    );
}

void VulkanCompute::createShaderModule(const std::string& file) {
    auto shaderCode = VulkanFrame::readFile(file);
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(shaderCode.data());
    shaderModuleCreateInfo.codeSize = shaderCode.size();

    if (vkCreateShaderModule(device, &shaderModuleCreateInfo,
            nullptr, &shaderModule
    ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }
}

void VulkanCompute::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[2] = {
        {
            0,// uint32_t              binding;
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,// VkDescriptorType      descriptorType;
            1,// uint32_t              descriptorCount;
            VK_SHADER_STAGE_COMPUTE_BIT,// VkShaderStageFlags    stageFlags;
            0// const VkSampler*      pImmutableSamplers;
        },
        {
            1,// uint32_t              binding;
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,// VkDescriptorType      descriptorType;
            1,// uint32_t              descriptorCount;
            VK_SHADER_STAGE_COMPUTE_BIT,// VkShaderStageFlags    stageFlags;
            0// const VkSampler*      pImmutableSamplers;
        }
    };
    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 2;
    createInfo.pBindings = descriptorSetLayoutBinding;

    if (vkCreateDescriptorSetLayout(
            device,&createInfo,
            nullptr, &descriptorSetLayout) != VK_SUCCESS
    ) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void VulkanCompute::createDescriptorPool() {
    VkDescriptorPoolSize descriptorPoolSize = {};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSize.descriptorCount = 2;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1; // we only need to allocate one descriptor set from the pool.
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

    vkCreateDescriptorPool(
        device,
        &descriptorPoolCreateInfo,
        nullptr,
        &descriptorPool
    );

}

void VulkanCompute::allocateDescriptorSet(){
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

    vkAllocateDescriptorSets(
        device,
        &descriptorSetAllocateInfo,
        &descriptorSet
    );
}

void VulkanCompute::connectBuffersToDescriptorSets() {
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
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,// VkStructureType                  sType;
            nullptr,// const void*                      pNext;
            descriptorSet,// VkDescriptorSet                  dstSet;
            0,// uint32_t                         dstBinding;
            0,// uint32_t                         dstArrayElement;
            1,// uint32_t                         descriptorCount;
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,// VkDescriptorType                 descriptorType;
            nullptr,// const VkDescriptorImageInfo*     pImageInfo;
            &inDBI,// const VkDescriptorBufferInfo*    pBufferInfo;
            nullptr// const VkBufferView*              pTexelBufferView;
        },
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,// VkStructureType                  sType;
            nullptr,// const void*                      pNext;
            descriptorSet,// VkDescriptorSet                  dstSet;
            1,// uint32_t                         dstBinding;
            0,// uint32_t                         dstArrayElement;
            1,// uint32_t                         descriptorCount;
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,// VkDescriptorType                 descriptorType;
            nullptr,// const VkDescriptorImageInfo*     pImageInfo;
            &resDBI,// const VkDescriptorBufferInfo*    pBufferInfo;
            nullptr// const VkBufferView*              pTexelBufferView;
        }
    };
    vkUpdateDescriptorSets(
        device,
        2,
        writeDescriptorSet,
        0,nullptr
    );
}

void VulkanCompute::createComputePipelineLayout() {
    VkPipelineLayoutCreateInfo pllci = {};
    pllci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pllci.setLayoutCount = 1;
    pllci.pSetLayouts = &descriptorSetLayout;
    vkCreatePipelineLayout(device,&pllci,nullptr,&pipelineLayout);
}

void VulkanCompute::createComputePipeline() {
    VkPipelineShaderStageCreateInfo ssci = {};
    ssci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ssci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    ssci.module = shaderModule;
    ssci.pName = "main";

    VkComputePipelineCreateInfo cplci = {};
    cplci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    cplci.stage = ssci;
    cplci.layout = pipelineLayout;

    vkCreateComputePipelines(
        device,
        VK_NULL_HANDLE,
        1,
        &cplci,nullptr,&pipeline
    );
}

void VulkanCompute::createCommandPool() {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = 0;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

    vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);
}

void VulkanCompute::allocateCommandBuffer() {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool; // specify the command pool to allocate from.
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1; // allocate a single command buffer.
    vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);
}

void VulkanCompute::recordCommands() {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    vkBeginCommandBuffer(commandBuffer,&beginInfo);

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        pipeline
    );
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        pipelineLayout,
        0,
        1,
        &descriptorSet,
        0,
        nullptr
    );
    vkCmdDispatch(
        commandBuffer,
        windowSize,// uint32_t groupCountX,
        1,// uint32_t groupCountY,
        1// uint32_t groupCountZ
    );

    vkEndCommandBuffer(commandBuffer);
}
