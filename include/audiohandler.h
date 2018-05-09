#pragma once

#include <vector>
#include <random>

#include <algorithm>

#include <iostream>
#include <fstream>

#include <limits>

#include <cassert>

#include <cmath>

#include <portaudio.h>

struct AudioHandler {
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dis;
    //std::normal_distribution<float> dis;
    int bufferSize;
    std::vector<char> data;
    std::vector<float> normData;
    std::vector<float> buffer;
    int overlap;

    AudioHandler(
            int bufferSize,
            int rate = 8000,
            int channels = 1
    ):
        gen(rd()),
        dis(-1.0,1.0),
        bufferSize(bufferSize),
        buffer(bufferSize),
        overlap(bufferSize * 0.925)
        //TODO: implement windowing algorithm somewhere (eg. Hanning)
    {
        if (Pa_Initialize() != paNoError) {
            throw std::runtime_error("Failed to initialize PortAudio!");
        }
        //getNormalizedMockAudio();
    }
    
    void generateTestAudio(
        size_t lenght,
        const std::vector<unsigned int>& frequencies,
        const std::vector<unsigned int>& amplitudes
    ) {
        assert( frequencies.size() == amplitudes.size() );
        normData.clear();
        normData.resize(lenght);
        for (int i = 0; i<normData.size(); ++i) {
            float t = (float)i/normData.size();        
            normData[i] = 0;
            for (int wave = 0; wave < frequencies.size(); ++wave){
                normData[i] += 
                    (float)amplitudes[wave] * 
                    (std::sin(t * frequencies[wave] * 2 * M_PI));
            }
        } 
        auto maxIt = std::max_element(normData.begin(),normData.end());
        float max = * maxIt;
        for (auto& d : normData){
            d /= max;
            //std::cout << d << ",";
        }
        //std::cout << std::endl;
    }

    void loadTestWAV(const std::string& filename) {
       std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: "+filename);
        }
        size_t fileSize = (size_t) file.tellg() - 44;
        data.resize(fileSize);
        file.seekg(44);
        file.read(data.data(), fileSize);
        file.close();
        normData.clear();
        for (char c : data) {
            normData.push_back(c / float(std::numeric_limits<char>::max()));
        }
    }

    const std::vector<float>& getNormalizedTestAudio() {
        std::copy(normData.begin(),normData.begin()+bufferSize, buffer.begin());
        normData.erase(normData.begin(),normData.begin()+(bufferSize-overlap));
        return buffer;
    }

    const std::vector<float>& getNormalizedMockAudio() {
        for (int i = 0; i < overlap; ++i) {
            buffer[i] = buffer[i+overlap];
        }
        for (int i = overlap; i<buffer.size(); ++i) {
            buffer[i] = dis(gen);
        }
        return buffer;
    }

    void startRecording();
};
