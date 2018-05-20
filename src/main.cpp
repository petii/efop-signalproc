
#include <iostream>
#include <stdexcept>

#include "visualizationapp.h"

#include <chrono>

int main (int argc, char ** argv) {
    try {
        int rangeBegin = 2;
        int rangeEnd = rangeBegin;
        if (argc == 2) {
            rangeBegin = std::stoi(std::string(argv[1]));
            rangeEnd = rangeBegin;
        }
        if (argc == 4) {
            rangeBegin = std::stoi(std::string(argv[1]));
            rangeEnd = std::stoi(std::string(argv[3]));
        }
        for (int multiplier = rangeBegin; multiplier<=rangeEnd; ++multiplier) {
            std::cout << std::string(50,'-') << std::endl;
            std::cout << "Base window size:\t" << VisualizationApplication::baseWindowSize << std::endl;
            std::cout << "Running with a base window size multiplier of " << multiplier << std::endl;
            std::cout << std::string(50,'-') << std::endl;
            auto initStart = 
                std::chrono::high_resolution_clock::now();
            VisualizationApplication app("Visualization with Vulkan",multiplier);
            auto initEnd = std::chrono::high_resolution_clock::now();
            std::cout << std::string(50,'-') << std::endl;
            std::cout << "Initialization took:\t" 
                << std::chrono::duration_cast<std::chrono::milliseconds>(initEnd-initStart).count()
                << " milliseconds\n"<< std::endl;

            if (argc > 2) {
                int runs = std::stoi(std::string(argv[2]));
                app.run(runs);
            }
            else {
                app.run();
            }
            std::cout << std::string(50,'-') << std::endl;
        }
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
