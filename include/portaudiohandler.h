#pragma once

#include <atomic>
#include <portaudio.h>
#include <thread>

#include "audiohandler.h"

class PortAudioHandler : public AudioHandler {
  PaStream *stream;

  static int recordCallback(const void *input, void *output,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo *timeInfo,
                            PaStreamCallbackFlags flags, void *data) {
    auto tp = std::chrono::high_resolution_clock::now();
    PortAudioHandler *ah = reinterpret_cast<PortAudioHandler *>(data);
    const double *dInput = static_cast<const double *>(input);
    if (input == nullptr) {
      ah->micInput->insert(ah->micInput->end(), frameCount, 0.0f);
    } else {
      ah->micInput->insert(ah->micInput->end(), dInput, dInput + frameCount);
    }
    if (!ah->measuring) {
      ah->startTime = tp;
      ah->measuring = true;
    }
    if (ah->micInput->size() >= ah->chunkSize) {
      std::thread([&ah](){ah->handler();});
    }
    return 0;
  }

public:
  const int framesPerBuffer = 256;
  size_t chunkSize;
  std::atomic<bool> measuring;
  std::chrono::high_resolution_clock::time_point startTime;
  std::chrono::high_resolution_clock::time_point endTime;
  std::vector<std::chrono::high_resolution_clock::duration> times;
  std::vector<double> buffer1;
  std::vector<double> buffer2;
  std::vector<double> *micInput;
  std::vector<double> *reserve;
  CallbackType subscriber;

  void handler() {
    // buffer pointed by micinput filled up
    auto tp = std::chrono::high_resolution_clock::now();
    auto *const tmp = micInput;
    micInput = reserve;
    reserve = tmp;
    subscriber(*reserve);
    reserve->clear();
    reserve->reserve(chunkSize);
    if (measuring) {
      endTime = tp;
      measuring = false;
      times.push_back(endTime-startTime);
    }
  }

  PortAudioHandler(CallbackType callback = CallbackType{},
                   unsigned channels = 1, unsigned sampleRate = 44100)
      : chunkSize(framesPerBuffer), measuring(false), buffer1(chunkSize),
        buffer2(chunkSize), micInput(&buffer1), reserve(&buffer2),
        subscriber(callback) {
    if (Pa_Initialize() != paNoError) {
      throw std::runtime_error("Failed to initialize PortAudio!");
    }
    PaStreamParameters inputParams = {};
    if ((inputParams.device = Pa_GetDefaultInputDevice()) == paNoDevice) {
      throw std::runtime_error("Could not get default input device!");
    }
    inputParams.channelCount = channels;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency =
        Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    if (Pa_OpenStream(&stream, &inputParams, nullptr, sampleRate,
                      framesPerBuffer, paClipOff,
                      PortAudioHandler::recordCallback, this) != paNoError) {
      throw std::runtime_error("Failed to open stream!");
    }
  }

  ~PortAudioHandler() {
    if (Pa_IsStreamActive(stream)) {
      Pa_CloseStream(stream);
    }
    Pa_Terminate();
  }

  bool startRecording() {
    if (Pa_StartStream(stream) != paNoError) {
      return false;
    }
    return true;
  }

  bool stopRecording() {
    if (Pa_IsStreamActive(stream) && Pa_StopStream(stream) == paNoError) {
      return true;
    }
    return false;
  }

  void setChunkSize(size_t newSize) { chunkSize = newSize; }
  void setCallback(CallbackType newCallback) { subscriber = newCallback; }

  const std::vector<std::chrono::high_resolution_clock::duration>& getMeasurements() {
    return times;
  }
};