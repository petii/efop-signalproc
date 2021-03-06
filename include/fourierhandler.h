#pragma once

#include <vector>

#define APPNAME "Fourier Transform"

class FourierHandler {

public:
  virtual ~FourierHandler() {};

  virtual void setWindowSize(size_t size) = 0;

  virtual void addInput(const std::vector<double> &input) = 0;

  virtual void runTransform() = 0;

  virtual std::vector<double> getResult() const = 0;
};