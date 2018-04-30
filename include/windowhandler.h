#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>

static void glfwErrorCallback(int error, const char* description){
    std::cout << "GLFW Error: " << error << " : " << description << std::endl;
}

struct WindowHandler {

    GLFWwindow* window;

    WindowHandler(
        const std::string& name,
        int width,
        int height,
        void* userPointer,
        GLFWwindowsizefun resizeFunction
    ) {
        std::cout << "window constructor\n";
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW!");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        //TODO: check for successful window creation
        window = glfwCreateWindow(
            width,
            height,
            name.c_str(),
            nullptr,
            nullptr
        );

        glfwSetWindowUserPointer(window,userPointer);
        glfwSetWindowSizeCallback(window,resizeFunction);

        glfwSetErrorCallback(glfwErrorCallback);
    }

    ~WindowHandler() {
        std::cout << "window destructor\n";
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    std::vector<const char*> getGLFWExtensions() const {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        std::cout << "extensions:\n";
        for (auto a : extensions){
            std::cout << a << std::endl;
        }
        return extensions;
    }

};
