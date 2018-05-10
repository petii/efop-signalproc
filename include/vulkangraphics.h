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
        void print() {
            std::cout << position.x << ',' << position.y << ',' << position.z << std::endl;;
        }

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

    VulkanGraphics(const VulkanFrame& vf, const WindowHandler& wh, int windowSize):
        vulkanFrame(&vf),
        qfi(vf.physicalDevice, vf.surface),
        rowSize(windowSize), //queue family indices
        //rowNum(128 )
        rowNum(100 ) //using this value does not reveal the lack of depth buffering...
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

        vkDestroyBuffer(device,vertexBuffer,nullptr);
        vkDestroyBuffer(device,indexBuffer,nullptr);
        vkFreeMemory(device,vertexBufferMemory,nullptr);
        vkFreeMemory(device,indexBufferMemory,nullptr);

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

    int rowSize;
    //how many rows should the drawing have
    int rowNum;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    void recordCommandBuffer();

    void drawFrame();

    void updateUniformBuffer();

    void appendVertices(std::vector<float> heights){
        //std::cout << vertices.size() << "/" << vertices.capacity()<<std::endl;
        //std::cin.get();
        if (vertices.size() == vertices.capacity()){
            //return;
            float distance = 
                std::abs(vertices[0].position.x - vertices[rowSize].position.x);
            vertices.erase(vertices.begin(),vertices.begin()+rowSize);
            indices.erase(indices.begin(),indices.begin()+(rowSize-1)*6);
            for (Vertex& v : vertices) {
                v.position.x -= distance;
            }
            for (auto& index : indices) {
                index -= rowSize;
            }
        } 
        int index = 0;
        int originalSize = vertices.size();
        for (float intensity : heights) {
            float time = (originalSize/(float)rowSize) / 20;
            float freq = (float)index/rowSize * 4 - 2.2f;
            Vertex v = {};
            //if (intensity > 1.0f) intensity /= 100;
            //else intensity *= 100;
            v.position = glm::vec3(time,freq,intensity /50);
            //v.position = glm::vec3(time,freq,0.0f);
            vertices.push_back(v);
            ++index;
        }
        //std::cout << "vertices orig size: " << originalSize << std::endl;
        if (originalSize == 0) return;
        for (int i = 0; i < rowSize-1 ; ++i) {
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
