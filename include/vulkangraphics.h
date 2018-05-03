#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "vulkanframe.h"
#include "windowhandler.h"

struct VulkanGraphics {
    struct QueueFamilyIndices {
        // int graphicsFamily = -1;
        // int presentFamily = -1;
        uint32_t graphicsFamily = -1;
        uint32_t presentFamily = -1;

        bool isComplete() {
            return graphicsFamily >= 0 && presentFamily >= 0;
        }
        QueueFamilyIndices(
            VkPhysicalDevice device,
            const VkSurfaceKHR& surface
        );
    };

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
            std::vector<VkVertexInputAttributeDescription> attrDescs;

            VkVertexInputAttributeDescription posAttrDesc = {};
            posAttrDesc.binding = 0;
            posAttrDesc.location = 0;
            posAttrDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
            posAttrDesc.offset = offsetof(Vertex, position);

            VkVertexInputAttributeDescription colAttrDesc = {};
            colAttrDesc.binding = 0;
            colAttrDesc.location = 1;
            colAttrDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
            colAttrDesc.offset = offsetof(Vertex, color);

            attrDescs.push_back(posAttrDesc);
            attrDescs.push_back(colAttrDesc);
            return attrDescs;
        }
    };

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    QueueFamilyIndices qfi;
    // VkPhysicalDevice physicalDevice;
    const VulkanFrame * vulkanFrame;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    // std::vector<VkCommandBuffer> commandBuffers;
    // since I record the commands at runtime probably one command buffer is enough
    VkCommandBuffer commandBuffer;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    VulkanGraphics(const VulkanFrame& vf, const WindowHandler& wh):
        vulkanFrame(&vf),
        qfi(vf.physicalDevice, vf.surface) //queue family indices
    {
        std::cout << "graphics constuctor\n";
        createGraphicsLogicalDevice(vf.physicalDevice,vf.deviceExtensions);
        vkGetDeviceQueue(device,qfi.graphicsFamily,0,&graphicsQueue);
        vkGetDeviceQueue(device,qfi.presentFamily,0,&presentQueue);
        createSwapChain(vf);
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createPipeLineLayout();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createVertexBuffer();
        createIndexBuffer();
        //Also allocates memory and binds them
        createUniformBuffer(vf.physicalDevice);
        createDescriptorPool();
        createDescriptorSet();
        createCommandBuffers();
        // will need to record the commands at runtime
        createSemaphores();
    }

    ~VulkanGraphics() {
        std::cout << "graphics destructor\n";
        vkDestroySemaphore(device, imageAvailableSemaphore,nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphore,nullptr);

        vkDestroyDescriptorPool(device,descriptorPool,nullptr);
        vkDestroyCommandPool(device,commandPool,nullptr);

        vkDestroyBuffer(device,uniformBuffer,nullptr);
        vkFreeMemory(device,uniformBufferMemory,nullptr);

        for (const auto& frameBuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device,frameBuffer,nullptr);
        }
        vkDestroyPipeline(device,graphicsPipeline,nullptr);
        vkDestroyPipelineLayout(device,pipelineLayout,nullptr);
        vkDestroyDescriptorSetLayout(device,descriptorSetLayout,nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        for (const auto& imageView : swapChainImageViews) {
            vkDestroyImageView(device,imageView,nullptr);
        }
        vkDestroySwapchainKHR(device,swapChain,nullptr);
        vkDestroyDevice(device,nullptr);
    }

    static int rowSize;

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    void recordCommandBuffer();

    void drawFrame();

    void updateUniformBuffer();

    void appendVertices(std::vector<float> heights = std::vector<float>(rowSize,0.0f)){
        int index = 0;
        int originalSize = vertices.size();
        for (const float& intensity : heights) {
            float time = std::floor(originalSize/rowSize) / 12;
            float freq = (float)index/rowSize * 2;
            Vertex v = {};
            v.position = glm::vec3(time,freq,intensity / 100 );
            //v.position = glm::vec3(time,freq,0.0f);
            vertices.push_back(v);
            ++index;
        }
        if (originalSize == 0) return;
        for (int i = 0; i < rowSize ; ++i) {
            indices.push_back(i+ originalSize );
            indices.push_back(i+ originalSize + 1 );
            indices.push_back(i+ originalSize - rowSize );

            indices.push_back(i+ originalSize + 1 );
            indices.push_back(i+ originalSize - rowSize + 1);
            indices.push_back(i+ originalSize - rowSize );
        }
    }
private:
    void createGraphicsLogicalDevice(const VkPhysicalDevice&,const std::vector<const char*>&);
    void createSwapChain(const VulkanFrame&);
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createPipeLineLayout();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffer(const VkPhysicalDevice&);
    void createDescriptorPool();
    void createDescriptorSet();
    void createCommandBuffers();
    void createSemaphores();
};
