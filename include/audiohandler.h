#pragma once

#include <vector>
#include <random>

#include <algorithm>

#include <iostream>
#include <fstream>

#include <limits>

struct AudioHandler {
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dis;
    //std::normal_distribution<float> dis;
    int bufferSize;
    std::vector<char> data;
    std::vector<float> normData;
    std::vector<float> buffer;
    static const int overlap = 0;

    AudioHandler(int bufferSize):
        gen(rd()),
        dis(-1.0,1.0),
        bufferSize(bufferSize),
        buffer(bufferSize) {
        
        getNormalizedMockAudio();
    }

    void loadTestAudio(const std::string& filename) {
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
};
