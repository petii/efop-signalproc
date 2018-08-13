#include "measurementsapp.h"

#include <chrono>
#include <fstream>
#include <memory>
#include <random>
#include <thread>

MeasurementsApp::MeasurementsApp(std::pair<int, int> range, int runs)
    : range(range), runs(runs) {}
MeasurementsApp::~MeasurementsApp() {}

void MeasurementsApp::exportResults(
    const std::string &file,
    const std::vector<std::vector<Measurement>> &mesr) {
  for (auto m : mesr) {
    std::ofstream out("./csvs/" + file + m.front().purpose + ".csv");
    for (auto &a : m) {
      out << a.execSize << (a.execSize != m.back().execSize ? "," : "");
    }
    out << std::endl;
    for (int i = 0; i < m.back().starts.size(); ++i) {
      for (auto &a : m) {
        out << std::chrono::duration_cast<std::chrono::nanoseconds>(a.ends[i] -
                                                                    a.starts[i])
                   .count()
            << (a.execSize != m.back().execSize ? "," : "");
      }
      out << std::endl;
    }
  }
}

void MeasurementsApp::doMeasurements() {
  std::clog << range.first << '-' << range.second << std::endl;
  std::clog << "portaudio" << std::endl;
  auto portAudioResults =
      runAudioMeasurements(std::make_unique<PortAudioHandler>());
  std::clog << "end of portaudio" << std::endl;
  //   auto pulseAudioResults =
  //       runAudioMeasurements(std::make_unique<PulseAudioHandler>());

  // std::clog << "running vulkan\n";
  // auto vulkanFourierResults =
  //     runFourierMeasurements(std::make_unique<VulkanFourier>());
  // std::clog << "end of vulkan\n";
  // std::clog << "running fftw++\n";
  // auto fftwFourierResults =
  //     runFourierMeasurements(std::make_unique<FFTWFourier>());
  // std::clog << "end of fftw++\n";
  // std::clog << "running fftw++ arrayless\n";
  // auto arrlessFourierResults =
  //     runFourierMeasurements(std::make_unique<ArraylessFFTW>());
  // std::clog << "end of fftw++ arrayless\n";
  // exportResults("vulkan", vulkanFourierResults);
  // exportResults("fftwpp", fftwFourierResults);
  // exportResults("arrless", arrlessFourierResults);
  // std::clog << "vulkan\t" << vulkanFourierResults.size() << std::endl;
  // for (int i = 0; i < vulkanFourierResults.size(); ++i) {
  //   for (auto &i : vulkanFourierResults[i]) {
  //     std::clog << i.toString() << std::endl;
  //   }
  //   std::clog << "fftwpp\t" << fftwFourierResults.size() << std::endl;
  //   for (auto &i : fftwFourierResults[i]) {
  //     std::clog << i.toString() << std::endl;
  //   }
  // }
}

std::vector<Measurement> MeasurementsApp::runAudioMeasurements(
    std::unique_ptr<AudioHandler> audioHandler) {
  for (int r = range.first; r <= range.second; ++r) {
    std::clog << r << std::endl;
    audioHandler->setChunkSize(r * baseWindowSize);
    int runNum = 0;
    std::vector<double> data;
    audioHandler->setCallback([&runNum,&data](auto audio) {
      data.insert(data.end(),audio.begin(),audio.end()); //TODO: move semantics?
      ++runNum;
    });
    // std::cin.get();
    //assert
    (audioHandler->startRecording());
    while (runNum < runs);
    //assert
    (audioHandler->stopRecording());
    auto results = audioHandler->getMeasurements();
  }
  return {};
}

std::vector<std::vector<Measurement>> MeasurementsApp::runFourierMeasurements(
    std::unique_ptr<FourierHandler> fourierHandler) {
  std::random_device randomDevice{};
  std::mt19937 generator(randomDevice());
  std::normal_distribution<> dist{};

  std::vector<std::vector<Measurement>> measurements;

  std::vector<Measurement> cmeasurements;
  std::vector<Measurement> rmeasurements;
  for (int size = range.first; size <= range.second; ++size) {
    auto dataSize = baseWindowSize * size;
    std::clog << dataSize << "\t";
    Measurement copyMeasurement("datacopy", runs, dataSize);
    Measurement runMeasurement("transformation", runs, dataSize);
    fourierHandler->setWindowSize(dataSize);
    std::vector<std::vector<double>> randomData;
    randomData.resize(runs);
    for (auto &data : randomData) {
      for (int i = 0; i < dataSize; ++i) {
        data.push_back(dist(generator));
      }
    }
    fourierHandler->addInput(randomData.front());
    fourierHandler->setWindowSize(dataSize);
    auto start = std::chrono::high_resolution_clock::now();
    for (auto &data : randomData) {
      copyMeasurement.add();
      fourierHandler->addInput(data);
      copyMeasurement.end();
      runMeasurement.add();
      // fourierHandler->runTransform();
      runMeasurement.end();
    }
    auto end = std::chrono::high_resolution_clock::now();
    copyMeasurement.overallRuntime = end - start;
    cmeasurements.push_back(copyMeasurement);
    rmeasurements.push_back(runMeasurement);
  }
  std::endl(std::clog);
  measurements.push_back(cmeasurements);
  measurements.push_back(rmeasurements);
  return measurements;
}
