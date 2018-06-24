
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "measurementsapp.h"

int main(int argc, char **argv) {
  if ( argc == 1 || (argc > 1 &&
      (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) ){
    std::clog
        << "usage:\t" << argv[0] << " "
        << "[#1 [#2] [#3]]" << std::endl
        << "Where:\n"
        << "\t#1: base windows size multiplier range start\n"
        << "\t#2: number of runs for each multiplier\n"
        << "\t#3: base windows size multiplier range end (default=start)\n";
    return 0;
  }
  try {
    std::pair<int, int> range{16, 16};
    if (argc >= 2) {
      range.first = std::stoi(std::string(argv[1]));
    }
    if (argc == 4) {
      range.second = std::stoi(std::string(argv[3]));
    } else {
      range.second = range.first;
    }

    MeasurementsApp measure(
        range, ((argc >= 3) ? std::stoi(std::string(argv[2])) : 1));
    measure.doMeasurements();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  return 0;
}
