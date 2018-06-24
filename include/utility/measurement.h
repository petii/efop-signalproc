#pragma once

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string>
#include <vector>

enum class MeasurementType {
  DataCopy,
  Transform,
};

struct Measurement {
  const int size;
  Measurement(MeasurementType type, int size) : size(size), purpose(type) {
    clear();
  }

  MeasurementType purpose;

  std::chrono::high_resolution_clock::duration overallRuntime;

  std::vector<std::chrono::high_resolution_clock::time_point> starts;
  std::vector<std::chrono::high_resolution_clock::time_point> ends;

  void clear() {
    overallRuntime = std::chrono::high_resolution_clock::duration::zero();
    starts.clear();
    starts.reserve(size);
    ends.clear();
    ends.reserve(size);
  }

  inline void add() {
    starts.push_back(std::chrono::high_resolution_clock::now());
  }

  inline void end() {
    ends.push_back(std::chrono::high_resolution_clock::now());
  }

  std::string toString() {
    std::stringstream result;
    result << "Overall runtime (" << size << " runs):\t"
           << std::chrono::duration_cast<std::chrono::milliseconds>(
                  overallRuntime)
                  .count()
           << " milliseconds" << std::endl;
    std::vector<long> diff;
    for (auto start = starts.begin(), end = ends.begin();
         start != starts.end() && end != ends.end(); ++start, ++end) {
      auto span = *end - *start;
      diff.push_back(
          std::chrono::duration_cast<std::chrono::milliseconds>(span).count());
    }
    result << "Average runtime:\t";
    // using custom loop instead of std::accumulate to avoid possible overflow
    double avg = 0.0;
    double N = static_cast<double>(diff.size());
    for (const auto &d : diff) {
      avg += d / N;
    }
    result << avg << " milliseconds\n";
    auto fastest = std::min_element(diff.begin(), diff.end());
    auto slowest = std::max_element(diff.begin(), diff.end());
    result << "Fastest runtime:\t" << *fastest << " milliseconds" << std::endl;
    result << "Slowest runtime:\t" << *slowest
           << " milliseconds"; // << std::endl;
    return result.str();
  }
};