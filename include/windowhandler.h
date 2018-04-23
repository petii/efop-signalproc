#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>

struct WindowHandler {

    GLFWwindow* window;

    WindowHandler(
        const std::string& name,
        int width,
        int height,
        void* userPointer,
        GLFWwindowsizefun resizeFunction
    ) {
        glfwInit();

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
    }

    ~WindowHandler() {
        std::cout << "window destructor\n";
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    std::vector<const char*> getGLFWExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        return extensions;
    }
};
