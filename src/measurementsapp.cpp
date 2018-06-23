#include "measurementsapp.h"

#include <random>

MeasurementsApp::MeasurementsApp(std::pair<int, int> range, int runs)
    : range(range), runs(runs) {}
MeasurementsApp::~MeasurementsApp() {}

void MeasurementsApp::doMeasurements() {
  auto portAudioResults =
      runAudioMeasurements(std::make_unique<PortAudioHandler>());
  auto pulseAudioResults =
      runAudioMeasurements(std::make_unique<PulseAudioHandler>());

  auto vulkanFourierResults =
      runFourierMeasurements(std::make_unique<VulkanFourier>());
  auto fftwFourierResults =
      runFourierMeasurements(std::make_unique<FFTWFourier>());
}

std::vector<Measurement> MeasurementsApp::runAudioMeasurements(
    std::unique_ptr<AudioHandler> audioHandler) {}
std::vector<Measurement> MeasurementsApp::runFourierMeasurements(
    std::unique_ptr<FourierHandler> fourierHandler) {
  std::random_device randomDevice{};
  std::mt19937 generator(randomDevice());
  std::normal_distribution<> dist{};

  std::vector<Measurement> measurements;

  for (int size = range.first; size < range.second; ++size) {
    auto dataSize = baseWindowSize * size;
    Measurement copyMeasurement(runs);
    Measurement runMeasurement(runs);
    fourierHandler->setWindowSize(dataSize);
    std::vector<std::vector<double>> randomData;
    randomData.resize(runs);
    for (auto &data : randomData) {
      for (int i = 0; i < dataSize; ++i) {
        data.push_back(dist(generator));
      }
    }
    fourierHandler->setWindowSize(dataSize);
    for (auto &data : randomData) {

      fourierHandler->addInput(data);
      fourierHandler->runTransform();
    }
    measurements.push_back(measurement);
  }
}
