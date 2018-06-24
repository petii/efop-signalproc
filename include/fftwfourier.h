#pragma once

#include "fourierhandler.h"

#include <memory>
#include <cassert>

#include "Array.h"
#include "fftw++.h"


class FFTWFourier : public FourierHandler {

  unsigned int windowSize;
  inline unsigned int resultSize() const { return windowSize / 2 + 1; }
  inline size_t align() const { return sizeof(Complex); }

  std::unique_ptr<fftwpp::rcfft1d> transform;

  std::unique_ptr<Array::array1<double>> data;
  std::unique_ptr<Array::array1<Complex>> result;
  std::vector<double> values;

  double hann(size_t index, double value) {
    auto w = 0.5 * (1 - std::cos( 2 * M_PI * index / (windowSize - 1) ) );
    return w * value;
  }

public:
  FFTWFourier() { fftwpp::fftw::maxthreads = get_max_threads(); }

  virtual void setWindowSize(size_t size) override {
    windowSize = size;
    data = std::make_unique < Array::array1<double>>(windowSize, align());
    result = std::make_unique < Array::array1<Complex>>(resultSize(), align());
    transform = std::make_unique<fftwpp::rcfft1d>(windowSize, *data, *result);
  };

  virtual void addInput(const std::vector<double> &input) override {
    assert(input.size() >= windowSize);
    auto &dataRef = *data;
    for (int i = 0; i < windowSize; ++i) {
      //apply hann function
      dataRef[i] = hann(i,input[i]);
    }
  }

  virtual void runTransform() override { 
    transform->fft(*data, *result); 
    values.clear();
    values.resize(resultSize());
    for (int i = 0; i < resultSize(); ++i) {
      auto real = (*result)[i].real();
      auto imag = (*result)[i].imag();
      values[i] = std::sqrt( real * real + imag * imag);
    }
  }

  virtual std::vector<double> getResult() const override {
    // TODO: implement if needed
    std::vector<double> ret(resultSize());
    for (int i = 0; i < resultSize(); ++i) {
      auto real = (*result)[i].real();
      auto imag = (*result)[i].imag();
      ret[i] = std::sqrt( real * real + imag * imag);
    }
    return ret;
  }
};