#pragma once

#include <complex>

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
        // while (!glfwWindowShouldClose(wh.window)) {
        //     glfwPollEvents();
        // }
        auto input = ah.getNormalizedMockAudio();
        vc.copyDataToGPU(input);
        vc.runCommandBuffer();
        // auto cpuDFT = discreteFourierTransformCPU(input);
        vkQueueWaitIdle(vc.queue);
        auto result = vc.readDataFromGPU();
        for (int i = 0 ; i < result.size() ; ++i) {
            if (result[i] == 0  && result[i+1] == 0) continue;
            std::cout << i/2 << '\t' << result[i] << '\t';
            std::cout << result[++i] << std::endl;
            // std::cout << i << '\t';
            // std::cout << result[i] << '\t' ;
            // if (i < input.size()) {
            //     std::cout << input[i] << '\t'
            //         << std::abs(result[i]-input[i]);
            // }
            // else {
            //     std::cout << result[++i] << '\t' ;
            // }
            // std::cout << std::endl;
        }
        // std::vector<std::complex<float>> gpuDFT(result.size()/2);
        // for (int i = 0; i<gpuDFT.size(); ++i) {
        //     gpuDFT[i] = std::complex<float>(result[2*i],result[2*i+1]);
        //     std::cout << i << '\t' << gpuDFT[i] << '\t' << std::abs(gpuDFT[i]) << std::endl;
        // }
        // std::cout << "\tGPU\t\t\tCPU\t\t\tDiff\n";
        // for (int i = 0, j=0 ; i<cpuDFT.size() && j<gpuDFT.size() ;  ++i , j++) {
        //     std::cout << i << '\t';
        //     std::cout << gpuDFT[j] << '\t';
        //     std::cout << cpuDFT[i] << '\t';
        //     std::cout << std::endl;
        // }
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
