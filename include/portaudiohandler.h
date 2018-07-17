#pragma once

#include <deque>
#include <random>
#include <vector>

#include <algorithm>

#include <fstream>
#include <iostream>

#include <limits>

#include <cassert>

#include <cmath>

#include <mutex>
#include <portaudio.h>

#include "audiohandler.h"

class PortAudioHandler : public AudioHandler {
  int overlap;
  std::deque<float> micInput;

  // TODO: use rate in file reading if needed
  int sampleRate;
  const int framesPerBuffer = 512;
  PaStreamParameters inputParams;
  PaStream *stream;

  std::mutex micMutex;

public:
  PortAudioHandler(int rate = 44100, int channels = 1) : sampleRate(rate) {
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

    if (Pa_OpenStream(&stream, &inputParams, nullptr, sampleRate,
                      framesPerBuffer, paClipOff, recordCallback,
                      this) != paNoError) {
      throw std::runtime_error("Failed to open stream!");
    }
    std::clog << "starting recording" << std::endl;
    startRecording();
    std::clog << "recording started" << std::endl;
  }

  ~PortAudioHandler() {
    if (Pa_IsStreamActive(stream)) {
      Pa_CloseStream(stream);
    }
    Pa_Terminate();
  }

  void startRecording() {
    if (Pa_StartStream(stream) != paNoError) {
      throw std::runtime_error("Failed to start recording audio!");
    }
  }

  std::mutex micSizeMutex;
  int micInputSize;

  static int recordCallback(const void *input, void *output,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo *timeInfo,
                            PaStreamCallbackFlags flags, void *data) {
    PortAudioHandler *ah = reinterpret_cast<PortAudioHandler *>(data);
    const double *dInput = static_cast<const double *>(input);
    std::lock_guard<std::mutex> lock(ah->micMutex);
    // std::clog << "hooray\n";
    if (input == nullptr) {
      ah->micInput.insert(ah->micInput.end(), frameCount, 0.0f);
    } else {
      ah->micInput.insert(ah->micInput.end(), dInput, dInput + frameCount);
    }
    if (ah->micSizeMutex.try_lock()) {
      ah->micInputSize = ah->micInput.size();
      ah->micSizeMutex.unlock();
    }
    ah->runs.push_back(std::chrono::high_resolution_clock::now());
    return 0;
  }

  int shift = 0;

  std::vector<double> getAudio(int items) {
    std::vector<double> buffer(items);
    while (micSizeMutex.try_lock() && micInputSize-shift < 2*items) {
      micSizeMutex.unlock();
      std::clog << "waiting";
      // usleep(10);
    }
    // std::lock_guard<std::mutex> lock(micMutex);
    std::cout << "hurray\n";
    buffer.assign(micInput.begin()+shift, micInput.begin()+shift + items);
    // micInput.erase(micInput.begin(), micInput.begin() + (items - overlap));
    shift+=items;
    return buffer;
  }
};
