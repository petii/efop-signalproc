#pragma once


#include <iostream>
#include "windowhandler.h"
#include "vulkanframe.h"
// #include "vulkancompute.h"
// #include "vulkangraphics.h"

class VisualizationApplication {
public:
    const int WIDTH = 800;
    const int HEIGHT = 600;

    const std::string name;
private:
    WindowHandler wh;

public:
    VisualizationApplication(const std::string& appName):
            name(appName),
            wh(
                name,
                WIDTH,
                HEIGHT,
                this,
                VisualizationApplication::onWindowResized
            ) {
        std::cout << "constructing\n";
        // wh.initWindow();
    }
    ~VisualizationApplication() {
        std::cout << "app destructor\n";
    }

    static void onWindowResized(GLFWwindow *window, int width, int height) {
        VisualizationApplication* app =
            reinterpret_cast<VisualizationApplication*>(glfwGetWindowUserPointer(window));
        // app->recreateSwapChain();
        int w,h;
        glfwGetWindowSize(app->wh.window, &w, &h);
        std::cout << app << ':' << w << 'x' << h << std::endl;
    }

    void run() {
        std::cout << "running running\n";
        mainLoop();
    }

    void mainLoop() {
        // while (!glfwWindowShouldClose(wh.window)) {
        //     glfwPollEvents();
        // }
    }
};
