#pragma once

#include <vector>
#include <deque>
#include <chrono>
#include <functional>

class AudioHandler {

public:
  const unsigned SAMPLE_RATE = 44100;
  const unsigned FRAMES_PER_BUFFER = 256;
  const unsigned CHANNELS = 1;

  using clock = std::chrono::high_resolution_clock;
  using CallbackType = std::function<void(const std::vector<double>&)>;

  virtual ~AudioHandler() {};

  virtual bool startRecording() = 0;
  virtual bool stopRecording() = 0;

  virtual void setChunkSize(size_t newSize) = 0;
  virtual void setCallback(CallbackType newCallback) = 0;

  virtual const std::vector<clock::duration>& getMeasurements() const = 0;
};