#pragma once

#include <complex>
#include <algorithm>

#include <iostream>
#include "windowhandler.h"
#include "vulkanframe.h"
#include "vulkancompute.h"
#include "vulkangraphics.h"
#include "audiohandler.h"

#include <chrono>

class VisualizationApplication {
public:
    const int WIDTH = 800;
    const int HEIGHT = 600;

    const std::string name;
//triple buffering megnezese
//osszehasonlitas

//teszteles helyett meresek
//tablazatok
//laptop, asztali gep osszehasonlitasa

    //this should be around 40k because it can only detect frequencies
    //between 0 and windowSize/2 (dft has real input, so the output is
    //"symmetric")
    const unsigned int windowSize = 4096 * 2 ;
    //const unsigned int windowSize = 8 ;
    const unsigned int freqDomainMax = windowSize/2;

    int numberOfRuns;
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
        //std::cout << "constructing\n";
        //VulkanGraphics::rowSize = vc.windowSize;
        //VulkanGraphics::rowSize = 4;
        // wh.initWindow();
    }
    ~VisualizationApplication() {
        //std::cout << "app destructor\n";
    }

    static void onWindowResized(GLFWwindow *window, int width, int height) {
        VisualizationApplication* app =
            reinterpret_cast<VisualizationApplication*>(glfwGetWindowUserPointer(window));
        //app->recreateSwapChain();
        int w,h;
        glfwGetWindowSize(app->wh.window, &w, &h);
        std::cout << app << ':' << w << 'x' << h << std::endl;
    }

    void run(int runNumbers = 100) {
        //std::cout << "running running\n";
        numberOfRuns = runNumbers;
        Measurements m(runNumbers);
        ah.startRecording();
        mainLoop(m);
        printMeasurements(m);
    }

private:

    struct Measurements{
        Measurements(int measurementNumber) {
            frameStarts.reserve(measurementNumber); 
            frameEnds.reserve(measurementNumber); 

            computeStarts.reserve(measurementNumber); 
            computeEnds.reserve(measurementNumber); 

            drawStarts.reserve(measurementNumber); 
            drawEnds.reserve(measurementNumber); 
        }

        std::chrono::duration<double> overallRuntime;
        std::vector<std::chrono::high_resolution_clock::time_point> frameStarts;
        std::vector<std::chrono::high_resolution_clock::time_point> frameEnds;

        std::vector<std::chrono::high_resolution_clock::time_point> computeStarts;
        std::vector<std::chrono::high_resolution_clock::time_point> computeEnds;

        std::vector<std::chrono::high_resolution_clock::time_point> drawStarts;
        std::vector<std::chrono::high_resolution_clock::time_point> drawEnds;
    };

    void printMeasurements(const Measurements& m) {
        std::cout << "The app ran for:\t" 
            << std::chrono::duration_cast<std::chrono::milliseconds>(m.overallRuntime).count() << " milliseconds\n" << std::endl;

    }


    void mainLoop(Measurements &m) {
        // Basic idea:
        //     get audio data
        //     feed data to computeDevice
        //     compute vertices
        //     create graphics from vertices
        // Sounds easy enough
        vg.appendVertices(std::vector<float>(freqDomainMax,0.0f));
        //ah.loadTestWAV("test/audio/a2002011001-e02-8kHz-mono.wav");

        //std::vector<unsigned int> freqs = {100,200,400,800,2080}; 
        //std::vector<unsigned int> amps = {5,4,3,2,1};
        //ah.generateTestAudio( windowSize , 16, freqs, amps );
        //std::vector<float> zeros (20,0.0f);
        int runTimes=0;
        auto overallRuntimeStart = std::chrono::high_resolution_clock::now();
        while (runTimes < numberOfRuns && !glfwWindowShouldClose(wh.window)) {
            //auto frameStart = 
            m.frameStarts.push_back(
                std::chrono::high_resolution_clock::now()
            );
            glfwPollEvents();

            auto input = ah.getMicrophoneAudio();

            //auto computeStart = 
            m.computeStarts.push_back(
                std::chrono::high_resolution_clock::now()
            );
            vc.copyDataToGPU(input);
            auto computeCopyEnd = 
                std::chrono::high_resolution_clock::now();
            auto computeRunStart =
                std::chrono::high_resolution_clock::now();
            vc.runCommandBuffer();
            vkQueueWaitIdle(vc.queue);
            //auto computeEnd =
            m.computeEnds.push_back(
                std::chrono::high_resolution_clock::now()
            );

            auto result = vc.readDataFromGPU();

            //auto drawStart = 
            m.drawStarts.push_back(
                std::chrono::high_resolution_clock::now()
            );
            vg.appendVertices(result);
            vg.updateUniformBuffer();
            vg.drawFrame(); 
            //This is unnecessary from a performance standpoint, and probably hurts
            vkQueueWaitIdle(vg.presentQueue);
            //auto drawEnd = 
            m.drawEnds.push_back(
                std::chrono::high_resolution_clock::now()
            );

            ++runTimes;
            //auto frameEnd = 
            m.frameEnds.push_back(
                std::chrono::high_resolution_clock::now()
            );
        }
        auto overallRuntimeEnd = std::chrono::high_resolution_clock::now();
        m.overallRuntime = 
            overallRuntimeEnd - overallRuntimeStart;
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
