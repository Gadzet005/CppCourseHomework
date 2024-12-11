#pragma once

#include <array>
#include <types/fixed.hpp>
#include <vector>

constexpr unsigned rhoSize = 256;
constexpr std::array<std::pair<int, int>, 4> deltas{
    {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

struct SimulationState {
    Fixed<> g;
    std::array<Fixed<>, rhoSize> rho;
    std::vector<std::vector<char>> field;

    size_t getFieldHeight() const { return field.size(); }
    size_t getFieldWidth() const {
        return field.size() > 0 ? field[0].size() : 0;
    }
};
