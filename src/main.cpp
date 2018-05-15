
#include <iostream>
#include <stdexcept>

#include "visualizationapp.h"

#include <chrono>

int main () {
    try {
        auto initStart = std::chrono::high_resolution_clock::now();
        VisualizationApplication app("Visualization with Vulkan");
        auto initEnd = std::chrono::high_resolution_clock::now();
        
        std::cout << "Initialization time:\t" 
            << std::chrono::duration_cast<std::chrono::milliseconds>(initEnd-initStart).count()
            << " milliseconds\n"<< std::endl;

        app.run();
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
