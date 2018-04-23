
#include <iostream>
#include <stdexcept>

#include "visualizationapp.h"

int main () {
    try {
        VisualizationApplication app("Visualization with Vulkan");

        app.run();
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
