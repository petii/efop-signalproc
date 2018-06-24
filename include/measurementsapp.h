#pragma once

#include <complex>
#include <iostream>
#include <memory>
#include <utility>

#include "audiohandler.h"
#include "fftwfourier.h"
#include "fourierhandler.h"
#include "utility/measurement.h"
#include "portaudiohandler.h"
#include "pulseaudiohandler.h"
#include "vulkanfourier.h"

class MeasurementsApp {
public:
  // meresek
  // tablazatok
  // laptop, asztali gep osszehasonlitasa

  static const int baseWindowSize = 256;
  unsigned int windowSize;
  inline unsigned int freqDomainMax() const { return windowSize / 2; }

  int runs;
  std::pair<int,int> range;

public:
  MeasurementsApp(std::pair<int, int> range, int runs);
  ~MeasurementsApp();

  void doMeasurements();

private:
  std::vector<Measurement>
  runAudioMeasurements(std::unique_ptr<AudioHandler> audioHandler);
  std::vector<Measurement>
  runFourierMeasurements(std::unique_ptr<FourierHandler> fourierHandler);

  std::vector<std::complex<float>>
  discreteFourierTransformCPU(std::vector<float> input) {
    std::vector<std::complex<float>> dft(input.size());
    for (size_t i = 0; i < dft.size(); ++i) {
      std::complex<float> x;
      for (size_t j = 0; j < input.size(); ++j) {
        double param = 2 * M_PI * i * j / (float)dft.size();
        x += input[j] * std::complex<float>(cos(param), -sin(param));
      }
      dft[i] = x;
    }
    return dft;
  }
};
