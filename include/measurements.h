#pragma once

#include <complex>
#include <iostream>

#include "audiohandler.h"
#include "fourierhandler.h"

#include "measurement.h"

class MeasurementsApp {
public:
  const std::string name;
  // triple buffering megnezese
  // osszehasonlitas

  // teszteles helyett meresek
  // tablazatok
  // laptop, asztali gep osszehasonlitasa

  static const int baseWindowSize = 256;
  // this should be around 40k because it can only detect frequencies
  // between 0 and windowSize/2 (dft has real input, so the output is
  //"symmetric")
  const unsigned int windowSize;
  unsigned int freqDomainMax() const { return windowSize / 2; }

  int numberOfRuns;

private:
  AudioHandler *audioHandler;
  FourierHandler *fourierHandler;

public:
  MeasurementsApp(const std::string &appName, int windowSizeMul)
      : name(appName) {}
  ~MeasurementsApp() {}

private:
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
