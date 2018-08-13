#pragma once

#include <vector>
#include <deque>
#include <chrono>
#include <functional>

class AudioHandler {

public:
  using CallbackType = std::function<void(const std::vector<double>&)>;

  virtual ~AudioHandler() {};

  virtual bool startRecording() = 0;
  virtual bool stopRecording() = 0;

  virtual void setChunkSize(size_t newSize) = 0;
  virtual void setCallback(CallbackType newCallback) = 0;

  virtual const std::vector<std::chrono::high_resolution_clock::duration>& getMeasurements() = 0;
};