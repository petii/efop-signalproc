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

    //this should be around 40k because it can only detect frequencies 
    //between 0 and windowSize/2 (dft has real input, so the output is
    //"symmetric")
    const unsigned int windowSize = 4096 * 1 ;
    //const unsigned int windowSize = 8 ;
    const unsigned int freqDomainMax = windowSize/2;
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
            vc(vf,windowSize), //VulkanCompute
            vg(vf,wh,freqDomainMax), //VulkanGraphics
            ah(windowSize) //audiohandler
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
        vg.appendVertices(std::vector<float>(freqDomainMax,0.0f));
        ah.loadTestWAV("test/audio/a2002011001-e02-8kHz-mono.wav");

        //std::vector<unsigned int> freqs = {100,200}; 
        //std::vector<unsigned int> amps(freqs.size(),1);
        std::vector<unsigned int> freqs = {100,200,400,800,2080}; 
        std::vector<unsigned int> amps = {5,4,3,2,1};
        //ah.generateTestAudio( windowSize , 16, freqs, amps );
        int runTimes=0;
        while (!glfwWindowShouldClose(wh.window)) {
            //std::cout << "Round " << runTimes << std::endl;
            glfwPollEvents();
            auto input = ah.getNormalizedTestAudio();
            //auto input = ah.getMicrophoneAudio();
            //auto input = normalizeResults(ah.getMicrophoneAudio());
            //for (auto i : input) {
            //    std::cout << i;
            //}
            //std::cin.get();
            vc.copyDataToGPU(input);
            vc.runCommandBuffer();
            //VkDeviceWaitIdle(vc.device);
            vkQueueWaitIdle(vc.queue);
            //auto result = vc.readDataFromGPU();
            auto result = normalizeResults(vc.readDataFromGPU());
            //result[0]=0.0f;
            /*for (int i = 0 ; i < result.size() ; ++i) {
                if (result[i] > 0.6f)
                std::cout << i << '/' << result.size() << ":\t" << result[i] << std::endl;
            }
            std::cout 
                << "min: " << *(std::min_element(result.begin(),result.end()))<< "\t"
                << "max: " << *(std::max_element(result.begin(),result.end())) << std::endl;
            */
            vg.appendVertices(result);
            vg.updateUniformBuffer();
            vg.drawFrame(); 
            /*if (ah.normData.size() < windowSize) {
                for (auto& f : freqs) { 
                    if ( f > 100 )
                        f -= 100 ;
                    else
                        f = freqDomainMax;
                }
                ah.generateTestAudio(windowSize,4,freqs, amps);
            }*/
            if (ah.normData.size() < windowSize) break;
            ++runTimes;
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
            for (size_t j = 0 ; j< input.size(); ++j) {
                double param = 2 * M_PI * i * j / (float)dft.size();
                x += input[j] * std::complex<float>(cos(param),-sin(param) );
            }
            dft[i] = x;
        }
        return dft;
    }
};
