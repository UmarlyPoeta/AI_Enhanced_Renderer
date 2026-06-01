#include "core/Application.hpp"

#include <iostream>
#include <stdexcept>

int main() {
    try {
        renderer::core::Application app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
