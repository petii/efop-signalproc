#pragma once

#include <complex>
#include <algorithm>

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
            vg(vf,wh,vc.windowSize), //VulkanGraphics
            ah(vc.windowSize) //audiohandler
    {
        std::cout << "constructing\n";
        //VulkanGraphics::rowSize = vc.windowSize;
        //VulkanGraphics::rowSize = 4;
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
        vg.appendVertices(std::vector<float>(vc.windowSize,0.0f));
        //vc.copyDataToGPU(ah.getNormalizedMockAudio());
        //vc.runCommandBuffer();
        //vkDeviceWaitIdle(vc.device);
        //auto result = normalizeResults(vc.readDataFromGPU());
        //vg.appendVertices(result);
        int runTimes=0;
        while (!glfwWindowShouldClose(wh.window)) {
            glfwPollEvents();
            // auto input = ah.getNormalizedMockAudio();
            // vc.copyDataToGPU(input);
            vc.copyDataToGPU(ah.getNormalizedMockAudio());
            vc.runCommandBuffer();
            vkDeviceWaitIdle(vc.device);
            //auto result = normalizeResults(vc.readDataFromGPU());
            auto result = vc.readDataFromGPU();
            //if (runTimes < 36)
                vg.appendVertices(result);
            vg.updateUniformBuffer();
            //if (runTimes < 200) {
            //vg.appendVertices();
            //}
            vg.drawFrame(); 
            //std::cout << runTimes;
            //std::cin.get();
            // for (int i = 0 ; i < result.size() ; ++i) {
            //     if (result[i] < 0.001f ) continue;
            //     std::cout << i << '\t' << result[i] << '\n';
            // }
            //break;
            ++runTimes;
            //if (runTimes >= 40) {
              //  std::cin.get();
                //break;
            //}
        }
    }

    std::vector<float> normalizeResults(const std::vector<float>& input) {
        auto max = std::max_element(input.begin(),input.end());
        std::vector<float> result;
        result.reserve(input.size());
        for (auto& item : input) {
            result.push_back(item / *max);
        } 
        return result;
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
