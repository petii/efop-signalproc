#pragma once

#include "audiohandler.h"

#include "portaudiocpp/PortAudioCpp.hxx"

inline void reset(std::chrono::high_resolution_clock::time_point &tp) {
  tp = std::chrono::high_resolution_clock::time_point();
}

class PACppHandler : public AudioHandler {

  using clock = std::chrono::high_resolution_clock;

  portaudio::AutoSystem autoSys;
  portaudio::System &system;

  std::unique_ptr<portaudio::MemFunCallbackStream<PACppHandler>> stream;

  CallbackType action;
  unsigned chunkSize = FRAMES_PER_BUFFER;

  std::unique_ptr<std::chrono::high_resolution_clock::time_point> firstBatch;
  std::vector<std::chrono::high_resolution_clock::duration> measures;

public:
  bool startRecording() override {
    measures.clear();
    buffer.clear();
    stream->start();
    return true;
  }

  bool stopRecording() override {
    stream->stop();
    return true;
  }

  void setChunkSize(size_t newSize) override {
    if (newSize % FRAMES_PER_BUFFER != 0) {
      newSize = (newSize / FRAMES_PER_BUFFER) * FRAMES_PER_BUFFER;
      std::clog << __FUNCTION__ << ": new chunksize not multiple of "
                << FRAMES_PER_BUFFER << "! Using size of " << newSize
                << " instead" << std::endl;
    }
    chunkSize = newSize;
  }

  void setCallback(CallbackType newCallback) override { action = newCallback; }

  const std::vector<std::chrono::high_resolution_clock::duration> &
  getMeasurements() const override {
    return measures;
  }

  PACppHandler()
      : system(portaudio::System::instance()), measures(), firstBatch(nullptr) {
    portaudio::DirectionSpecificStreamParameters inParams(
        system.defaultInputDevice(), CHANNELS, portaudio::FLOAT32, false,
        system.defaultInputDevice().defaultLowInputLatency(), nullptr);
    portaudio::DirectionSpecificStreamParameters outParams(
        system.defaultOutputDevice(), CHANNELS, portaudio::FLOAT32, false,
        system.defaultOutputDevice().defaultLowOutputLatency(), nullptr);
    portaudio::StreamParameters params(
        inParams, portaudio::DirectionSpecificStreamParameters::null(),
        SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff);

    stream = std::make_unique<portaudio::MemFunCallbackStream<PACppHandler>>(
        params, *this, &PACppHandler::callback);
  }

  std::vector<double> buffer;

  int callback(const void *inputBuffer, void *outputBuffer,
               unsigned long framesPerBuffer,
               const PaStreamCallbackTimeInfo *timeInfo,
               PaStreamCallbackFlags flags) {
    if (firstBatch == nullptr) {
      firstBatch = std::make_unique<clock::time_point>(clock::now());
    }
    auto input = static_cast<const float *>(inputBuffer);
    for (unsigned i = 0; i < framesPerBuffer; ++i) {
      buffer.push_back(input[i]);
    }
    if (buffer.size() >= chunkSize) {
      action(buffer); // TODO: move semantics
      buffer.clear();
      measures.push_back(clock::now() - *firstBatch);
      firstBatch = nullptr;
    }
    return paContinue;
  }
};
