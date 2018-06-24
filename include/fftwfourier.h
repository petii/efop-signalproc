#pragma once

#include "fourierhandler.h"

#include "Array.h"
#include "fftw++.h"

class FFTWFourier : public FourierHandler {

  int windowSize;

public:
  FFTWFourier() {
    fft
  }

  virtual void setWindowSize(size_t size) override {
    windowSize = size;
  };

  virtual void addInput(const std::vector<double> &input) override {

  }

  virtual void runTransform() override {

  }

  virtual std::vector<double> getResult() const override {
    //TODO: implement if needed
  }

}; 