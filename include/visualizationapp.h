#pragma once


#include <iostream>
#include "windowhandler.h"
#include "vulkanframe.h"
#include "vulkancompute.h"
// #include "vulkangraphics.h"
#include "audiohandler.h"

class VisualizationApplication {
public:
    const int WIDTH = 800;
    const int HEIGHT = 600;

    const std::string name;
private:
    WindowHandler wh;
    VulkanFrame vf;
    VulkanCompute vc;
    // VulkanGraphics vg;
    AudioHandler ah;

public:
    VisualizationApplication(const std::string& appName):
            name(appName),
            wh( //WindowHandler
                name,
                WIDTH,
                HEIGHT,
                this,
                VisualizationApplication::onWindowResized
            ),
            vf( //VulkanFrame
                name,
                wh.getGLFWExtensions()
            ),
            vc(vf), //VulkanCompute
            ah(vc.bufferSize) //audiohandler
    {
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
        // Basic idea:
        //     get audio data
        //     feed data to computeDevice
        //     compute vertices
        //     create graphics from vertices
        // Sounds easy enough
        // while (!glfwWindowShouldClose(wh.window)) {
        //     glfwPollEvents();
        // }
        vc.copyData(ah.getNormalizedMockAudio());
        vc.runCommandBuffer();
    }
};
