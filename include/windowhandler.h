#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

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
};
