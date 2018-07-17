#pragma once

#include <vector>
#include <deque>
#include <chrono>

class AudioHandler {

public:
  virtual ~AudioHandler() {};

  std::deque<std::chrono::high_resolution_clock::time_point> runs;

  virtual std::vector<double> getAudio(int items) = 0;
};