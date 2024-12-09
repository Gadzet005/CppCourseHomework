#pragma once

#include <cstddef>
#include <fstream>
#include <limits>
#include <simulation/common.hpp>

SimulationDescription loadSimulationDescription(const string &path) {
    SimulationDescription description;

    ifstream input;

    input.open(path);
    if (!input.is_open()) {
        throw runtime_error("Unable to open file");
    }

    input >> description.g;

    size_t rhoCount;
    input >> rhoCount;

    for (size_t i = 0; i < rhoCount; ++i) {
        input.ignore(numeric_limits<streamsize>::max(), '\n');
        char c = input.get();
        input >> description.rho[static_cast<size_t>(c)];
    }

    size_t height, width;
    input >> height >> width;

    description.field.resize(height, vector<char>(width));
    for (size_t x = 0; x < height; ++x) {
        input.ignore(numeric_limits<streamsize>::max(), '\n');
        for (size_t y = 0; y < width; ++y) {
            description.field[x][y] = input.get();
        }
    }

    return description;
}