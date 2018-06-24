#pragma once

#include <vector>

class AudioHandler {

public:
  virtual std::vector<double> getAudio(int items) = 0;
};