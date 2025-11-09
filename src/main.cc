// main.cc
#include "engine.h"
#include <filesystem>
#include <iostream>
#include <memory>

auto main() -> int {
    std::cout << std::filesystem::current_path() << std::endl;

    std::unique_ptr<Engine> engine =
        std::make_unique<Engine>(800, 600, "Voxels");

    engine->init();
    engine->run();


    return EXIT_SUCCESS;
}
