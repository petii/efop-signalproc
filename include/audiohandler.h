#pragma once

#include <deque>
#include <vector>
#include <random>

#include <algorithm>

#include <iostream>
#include <fstream>

#include <limits>

#include <cassert>

#include <cmath>

#include <portaudio.h>
#include <mutex>

class AudioHandler {
    //std::random_device rd;
    //std::mt19937 gen;
    //std::uniform_real_distribution<float> dis;
    //std::normal_distribution<float> dis;
    int bufferSize;
    std::vector<char> data;
    std::vector<float> normData;
    std::vector<float> buffer;
    int overlap;
    std::deque<float> micInput;

    int sampleRate;
    const int framesPerBuffer = 512;
    PaStreamParameters inputParams;
    PaStream *stream;

    std::mutex micMutex;
public:
    AudioHandler(
            int bufferSize,
            int rate = 44100,
            int channels = 1
    ):
        //gen(rd()),
        //dis(-1.0,1.0),
        bufferSize(bufferSize),
        buffer(bufferSize),
        overlap(bufferSize * 0.2f),
        //TODO: implement windowing algorithm somewhere (eg. Hanning)
        //micInput(bufferSize,0.0f),
        sampleRate(rate)
    {
        //micInput.reserve(2*bufferSize);
        if (Pa_Initialize() != paNoError) {
            throw std::runtime_error("Failed to initialize PortAudio!");
        }

        if ((inputParams.device = Pa_GetDefaultInputDevice()) == paNoDevice) {
            throw std::runtime_error("Could not get default input device!");
        }
        inputParams.channelCount = channels;
        inputParams.sampleFormat = paFloat32;
        inputParams.suggestedLatency = 
            Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
        inputParams.hostApiSpecificStreamInfo = nullptr;

        if (Pa_OpenStream(
            &stream,
            &inputParams,
            nullptr,
            sampleRate,
            framesPerBuffer,
            paClipOff,
            recordCallback,
            this
        ) != paNoError) {
            throw std::runtime_error("Failed to open stream!");
        }
    }
    ~AudioHandler() {
        if (Pa_IsStreamActive(stream)) {
            Pa_CloseStream(stream);
        }
        Pa_Terminate();
    }
    
    void generateTestAudio(
        size_t lenght,
        const std::vector<unsigned int>& frequencies,
        const std::vector<unsigned int>& amplitudes
    ) {
        assert( frequencies.size() == amplitudes.size() );
        normData.clear();
        normData.resize(lenght);
        for (size_t i = 0; i<normData.size(); ++i) {
            float t = (float)i/normData.size();        
            normData[i] = 0;
            for (size_t wave = 0; wave < frequencies.size(); ++wave){
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
/*
    const std::vector<float>& getNormalizedMockAudio() {
        for (int i = 0; i < overlap; ++i) {
            buffer[i] = buffer[i+overlap];
        }
        for (size_t i = overlap; i<buffer.size(); ++i) {
            buffer[i] = dis(gen);
        }
        return buffer;
    }

    void startRecording() {
        if (Pa_StartStream(stream) != paNoError) {
            throw std::runtime_error("Failed to start recording audio!");
        }
    }
//*/

    static int recordCallback(
        const void* input,
        void* output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags flags,
        void* data
    ) {
        AudioHandler* ah = reinterpret_cast<AudioHandler*>(data); 
        //std::cout << "recordCallback\t" << frameCount << "\t" << *(const float*)input << " " << ah->micInput.size() << std::endl;
        const float* fInput = (float*)input;
        std::lock_guard<std::mutex> lock(ah->micMutex);
        //std::cout << "hooray\n";
        if (input == nullptr) {
            ah->micInput.insert(ah->micInput.end(),frameCount,0.0f);
        }
        else {
            for (int i = 0; i < frameCount; ++i) {
                ah->micInput.push_back(*fInput);
                ++fInput;
            }
        }
        return 0;
    }

    std::vector<float> getMicrophoneAudio() {
        std::lock_guard<std::mutex> lock(micMutex);
        //std::cout << "hurray\n";
        if (micInput.size() < bufferSize) {
            return buffer;
        }
        else {
            buffer.assign(micInput.begin(),micInput.begin()+bufferSize);
            micInput.erase(micInput.begin(),micInput.begin()+(bufferSize-overlap));
        }
        return buffer;
    }
};
