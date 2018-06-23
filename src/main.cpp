
#include <iostream>
#include <stdexcept>
#include <string>

#include "measurements.h"

int main (int argc, char ** argv) {
    if (argc > 1 && ( std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") ) {
        std::clog << "usage:\t" << argv[0] << " " 
                  << "[#1 [#2] [#3]]" << std::endl
                  << "Where:\n" 
                  << "\t#1: base windows size multiplier range start\n"
                  << "\t#2: number of runs for each multiplier\n"
                  << "\t#3: base windows size multiplier range end (default=start)\n";
        return 0;
    } 
    try {
        int rangeBegin = 16;
        if (argc == 2) {
            rangeBegin = std::stoi(std::string(argv[1]));
        }
        int rangeEnd = rangeBegin;
        if (argc == 4) {
            rangeBegin = std::stoi(std::string(argv[1]));
            rangeEnd = std::stoi(std::string(argv[3]));
        }
        for (int multiplier = rangeBegin; multiplier<=rangeEnd; ++multiplier) {
            std::clog << std::string(50,'-') << std::endl;
            // std::clog << "Base window size:\t" << VisualizationApplication::baseWindowSize << std::endl;
            // std::clog << "Running with a base window size multiplier of " << multiplier << std::endl;
            // std::clog << std::string(50,'-') << std::endl;
            // auto initStart = 
            //     std::chrono::high_resolution_clock::now();
            // VisualizationApplication app("Visualization with Vulkan",multiplier);
            // auto initEnd = std::chrono::high_resolution_clock::now();
            // std::clog << std::string(50,'-') << std::endl;
            // std::clog << "Initialization took:\t" 
            //     << std::chrono::duration_cast<std::chrono::milliseconds>(initEnd-initStart).count()
            //     << " milliseconds\n"<< std::endl;

            // if (argc > 2) {
            //     int runs = std::stoi(std::string(argv[2]));
            //     app.run(runs);
            // }
            // else {
            //     app.run();
            // }
            // std::clog << std::string(50,'-') << std::endl;
        }
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}
