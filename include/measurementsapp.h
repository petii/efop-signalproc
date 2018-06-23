#pragma once

#include <complex>
#include <iostream>
#include <utility>
#include <memory>

#include "audiohandler.h"
#include "fourierhandler.h"
#include "portaudiohandler.h"
#include "pulseaudiohandler.h"
#include "vulkanfourier.h"
#include "fftwfourier.h"
#include "measurement.h"

class MeasurementsApp {
public:
  // meresek
  // tablazatok
  // laptop, asztali gep osszehasonlitasa

  static const int baseWindowSize = 256;
  unsigned int windowSize;
  unsigned int freqDomainMax() const { return windowSize / 2; }

  int numberOfRuns;

private:
  AudioHandler *audioHandler;
  FourierHandler *fourierHandler;

public:
  MeasurementsApp(std::pair<int,int> range, int runs) {}
  ~MeasurementsApp() {}

  void doMeasurements() {
    auto portAudioResults = runAudioMeasurements(std::make_unique<PortAudioHandler>(...));
    auto pulseAudioResults = runAudioMeasurements(std::make_unique<PulseAudioHandler>(...));

    auto vulkanFourierResults = runFourierMeasurements(std::make_unique<VulkanFourier>(...));
    auto fftwFourierResults = runFourierMeasurements(std::make_unique<FFTWFourier>(...));
  }
private:
  Measurement runAudioMeasurements(std::unique_ptr<AudioHandler> audioHandler);
  Measurement runFourierMeasurements(std::unique_ptr<FourierHandler> fourierHandler);

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
