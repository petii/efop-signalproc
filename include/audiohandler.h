#pragma once

#include <vector>
#include <random>

struct AudioHandler {
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dis;
    int bufferSize;
    std::vector<float> buffer;

    AudioHandler(int bufferSize):
        gen(rd()),
        dis(-1.0,1.0),
        bufferSize(bufferSize),
        buffer(bufferSize) {}

    const std::vector<float>& getNormalizedMockAudio() {
        for (int i = 0; i<buffer.size(); ++i) {
            buffer[i] = dis(gen);
        }
        return buffer;
    }
};
