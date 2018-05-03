#pragma once

#include <vector>
#include <random>

#include <algorithm>

struct AudioHandler {
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dis;
    //std::normal_distribution<float> dis;
    int bufferSize;
    std::vector<float> buffer;
    static const int overlap = 0;

    AudioHandler(int bufferSize):
        gen(rd()),
        dis(-1.0,1.0),
        bufferSize(bufferSize),
        buffer(bufferSize) {
        
        getNormalizedMockAudio();
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
