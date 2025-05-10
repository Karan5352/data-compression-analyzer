#include "gui/MainWindow.h"
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
    try {
        MainWindow window;
        window.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
} 