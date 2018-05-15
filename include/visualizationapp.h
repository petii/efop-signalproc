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

    static const int baseWindowSize = 2048;
    //this should be around 40k because it can only detect frequencies
    //between 0 and windowSize/2 (dft has real input, so the output is
    //"symmetric")
    const unsigned int windowSize;
    unsigned int freqDomainMax() const {return windowSize/2;}

    int numberOfRuns;
private:
    WindowHandler wh;
    VulkanFrame vf;
    VulkanCompute vc;
    VulkanGraphics vg;
    AudioHandler ah;

public:
    VisualizationApplication(const std::string& appName, int windowSizeMul):
            name(appName),
            windowSize(baseWindowSize * windowSizeMul),
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
            vg(vf,wh,freqDomainMax()), //VulkanGraphics
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
    /*
        VisualizationApplication* app =
            reinterpret_cast<VisualizationApplication*>(glfwGetWindowUserPointer(window));
        //app->recreateSwapChain();
        int w,h;
        glfwGetWindowSize(app->wh.window, &w, &h);
        std::cout << app << ':' << w << 'x' << h << std::endl;
    //*/
    }

    void run(int runNumbers) {
        //std::cout << "running running\n";
        numberOfRuns = runNumbers;
        Measurements m(runNumbers);

        //fill up the vertex buffer with zeros for accurate measurement
        for (int i = 0; i < vg.rowNum; ++i) {
            vg.appendVertices(std::vector<float>(freqDomainMax(),0.0f));
        }
        ah.startRecording();
        //mainLoop(m);
        std::cout << "Measuring time to draw " << runNumbers << " frames:\t";
        auto runTime = mainLoopMeasureRunTime();
        auto runTimeInMillisecs = std::chrono::duration_cast<std::chrono::milliseconds>(runTime).count();
        std::cout << runTimeInMillisecs << " milliseconds\n";
        std::cout << "Calculated average for 1 frame:\t" 
            << static_cast<double>(runTimeInMillisecs) / numberOfRuns 
            << " milliseconds" <<  std::endl;
        std::cout << std::endl;
        std::cout << "Measuring the time it takes to draw a frame.\n";
        mainLoopMeasureFrameTimes(m);
        std::cout << m.toString() << std::endl;
        m.clear();
        std::cout << std::endl;
        std::cout << "Measuring time it takes to complete the compute task.\n";
        mainLoopMeasureComputeTime(m);
        std::cout << m.toString() << std::endl;
        m.clear();
        std::cout << std::endl;
        std::cout << "Measuring time it takes to complete the drawing task.\n";
        mainLoopMeasureDrawTime(m);
        std::cout << m.toString() << std::endl;
    }

    void run() {
        ah.startRecording();
        mainLoop();
    }

private:

    struct Measurements{
        const int size;
        Measurements(int size):size(size) {
            clear();
        }

        //std::chrono::duration<long> overallRuntime;
        std::chrono::high_resolution_clock::duration overallRuntime;

        std::vector<std::chrono::high_resolution_clock::time_point> starts;
        std::vector<std::chrono::high_resolution_clock::time_point> ends;
        void clear() {
            overallRuntime = std::chrono::duration<long>::zero();
            starts.clear();
            starts.resize(size);
            ends.clear();
            ends.resize(size);
        }

        std::string toString() {
            std::stringstream result;
            result << "Overall runtime (" << size << " runs):\t" 
                << std::chrono::duration_cast<std::chrono::milliseconds>(overallRuntime).count() << " milliseconds" << std::endl;   
            std::vector<long> diff;
            for (
                auto start = starts.begin(), end=ends.begin() ; 
                start!=starts.end() && end!=ends.end();
                ++start,++end
            ){
                auto span = *end - *start;
                diff.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(span).count());
            }
            result << "Average runtime:\t";
            //using custom loop instead of std::accumulate to avoid possible overflow
            double avg = 0.0;
            double N = static_cast<double>(diff.size());
            for (const auto& d : diff) {
                avg += d/N; 
            }
            result << avg << " milliseconds\n";
            auto fastest = std::min_element(diff.begin(),diff.end());
            auto slowest = std::max_element(diff.begin(),diff.end());
            result << "Fastest runtime:\t" << *fastest 
                << " milliseconds" << std::endl;
            result << "Slowest runtime:\t" << *slowest 
                << " milliseconds" ;// << std::endl; 
            return result.str();
        }
    };

    void mainLoop() {
        // Basic idea:
        //     get audio data
        //     feed data to computeDevice
        //     compute vertices
        //     create graphics from vertices
        // Sounds easy enough
        vg.appendVertices(std::vector<float>(freqDomainMax(),0.0f)); // so we can start drawing right away
        while (!glfwWindowShouldClose(wh.window)) {
            glfwPollEvents();
            auto input = ah.getMicrophoneAudio();
            vc.copyDataToGPU(input);
            vc.runCommandBuffer();
            vkQueueWaitIdle(vc.queue);
            auto result = vc.readDataFromGPU();
            vg.appendVertices(result);
            vg.updateUniformBuffer();
            vg.drawFrame();
        }
    }

    std::chrono::high_resolution_clock::duration mainLoopMeasureRunTime(/*std::chrono::duration<long>& m*/) {
        int runTimes=0;
        auto overallRuntimeStart = std::chrono::high_resolution_clock::now();
        while (runTimes < numberOfRuns && !glfwWindowShouldClose(wh.window)) {
            glfwPollEvents();
            auto input = ah.getMicrophoneAudio();
            vc.copyDataToGPU(input);
            vc.runCommandBuffer();
            vkQueueWaitIdle(vc.queue);
            auto result = vc.readDataFromGPU();
            vg.appendVertices(result);
            vg.updateUniformBuffer();
            vg.drawFrame();
            ++runTimes;
        }
        auto overallRuntimeEnd = std::chrono::high_resolution_clock::now();
        return overallRuntimeEnd - overallRuntimeStart;
    }

    void mainLoopMeasureFrameTimes(Measurements &m) {
        int runTimes=0;
        auto overallRuntimeStart = std::chrono::high_resolution_clock::now();
        while (runTimes < numberOfRuns && !glfwWindowShouldClose(wh.window)) {
            //auto frameStart = 
            m.starts[runTimes] = 
                std::chrono::high_resolution_clock::now();
            glfwPollEvents();
            auto input = ah.getMicrophoneAudio();
            vc.copyDataToGPU(input);
            vc.runCommandBuffer();
            vkQueueWaitIdle(vc.queue);
            auto result = vc.readDataFromGPU();
            vg.appendVertices(result);
            vg.updateUniformBuffer();
            vg.drawFrame(); 
            //auto frameEnd = 
            vkQueueWaitIdle(vg.presentQueue);
            m.ends[runTimes] = 
                std::chrono::high_resolution_clock::now();
            ++runTimes;
        }
        auto overallRuntimeEnd = std::chrono::high_resolution_clock::now();
        m.overallRuntime = 
            overallRuntimeEnd - overallRuntimeStart;
    }
void mainLoopMeasureComputeTime(Measurements &m) {
        int runTimes=0;
        auto overallRuntimeStart = std::chrono::high_resolution_clock::now();
        while (runTimes < numberOfRuns && !glfwWindowShouldClose(wh.window)) {
            glfwPollEvents();

            auto input = ah.getMicrophoneAudio();

            //auto computeStart = 
            m.starts[runTimes] = 
                std::chrono::high_resolution_clock::now();
            vc.copyDataToGPU(input);
            //auto computeCopyEnd = 
            //    std::chrono::high_resolution_clock::now();
            //auto computeRunStart =
            //    std::chrono::high_resolution_clock::now();
            vc.runCommandBuffer();
            vkQueueWaitIdle(vc.queue);
            //auto computeEnd =
            m.ends[runTimes] = 
                std::chrono::high_resolution_clock::now();
            auto result = vc.readDataFromGPU();
            vg.appendVertices(result);
            vg.updateUniformBuffer();
            vg.drawFrame(); 
            ++runTimes;
        }
        auto overallRuntimeEnd = std::chrono::high_resolution_clock::now();
        m.overallRuntime = 
            overallRuntimeEnd - overallRuntimeStart;
    }
void mainLoopMeasureDrawTime(Measurements &m) {
        int runTimes=0;
        auto overallRuntimeStart = std::chrono::high_resolution_clock::now();
        while (runTimes < numberOfRuns && !glfwWindowShouldClose(wh.window)) {
            glfwPollEvents();
            auto input = ah.getMicrophoneAudio();
            vc.copyDataToGPU(input);
            vc.runCommandBuffer();
            vkQueueWaitIdle(vc.queue);
            auto result = vc.readDataFromGPU();
            //auto drawStart = 
            m.starts[runTimes] = 
                std::chrono::high_resolution_clock::now();
            vg.appendVertices(result);
            vg.updateUniformBuffer();
            vg.drawFrame(); 
            //Drawing and presenting are both async operations.
            //To accurately measure how long it takes to actually 
            //draw the frame we need to be done with presenting.
            //
            //This negatively impacts performance, since we can not 
            //start the Fourier-transform while presenting is still
            //in progess 
            vkQueueWaitIdle(vg.presentQueue);
            //auto drawEnd = 
            m.ends[runTimes] = 
                std::chrono::high_resolution_clock::now();
            ++runTimes;
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
