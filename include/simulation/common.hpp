#pragma once

#include <array>
#include <types/fixed.hpp>
#include <vector>

constexpr unsigned rhoSize = 256;
constexpr std::array<pair<int, int>, 4> deltas{
    {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

struct SimulationDescription {
    Fixed<> g;
    array<Fixed<>, rhoSize> rho;
    vector<vector<char>> field;

    SimulationDescription() = default;
    size_t getHeight() const { return field.size(); }
    size_t getWidth() const { return field.size() > 0 ? field[0].size() : 0; }
};
