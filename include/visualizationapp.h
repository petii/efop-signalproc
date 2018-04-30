#pragma once

#include <complex>

#include <iostream>
#include "windowhandler.h"
#include "vulkanframe.h"
#include "vulkancompute.h"
#include "vulkangraphics.h"
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
    VulkanGraphics vg;
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
                wh
            ),
            vc(vf), //VulkanCompute
            vg(vf,wh), //VulkanGraphics
            ah(vc.windowSize) //audiohandler
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
        while (!glfwWindowShouldClose(wh.window)) {
            glfwPollEvents();
            // auto input = ah.getNormalizedMockAudio();
            // vc.copyDataToGPU(input);
            vc.copyDataToGPU(ah.getNormalizedMockAudio());
            vc.runCommandBuffer();
            vkQueueWaitIdle(vc.queue);
            auto result = vc.readDataFromGPU();
            for (int i = 0 ; i < result.size() ; ++i) {
                if (result[i] < 0.001f ) continue;
                std::cout << i << '\t' << result[i] << '\n';
            }
            break;
        }
    }

    std::vector<std::complex<float>> discreteFourierTransformCPU(std::vector<float> input) {
        std::vector<std::complex<float>> dft(input.size());
        for (size_t i = 0; i<dft.size(); ++i) {
            std::complex<float> x;
            for (int j = 0 ; j< input.size(); ++j) {
                double param = 2 * M_PI * i * j / (float)dft.size();
                x += input[j] * std::complex<float>(cos(param),-sin(param) );
            }
            dft[i] = x;
        }
        return dft;
    }
};
