#pragma once

#include <array>
#include <types/fixed.hpp>
#include <vector>

constexpr unsigned rhoSize = 256;
constexpr std::array<std::pair<int, int>, 4> deltas{
    {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

template <typename T, size_t N, size_t M>
using StaticMatrix = std::array<std::array<T, M>, N>;

template <typename T>
using DynamicMatrix = std::vector<std::vector<T>>;

template <typename T, size_t N, size_t M>
using StaticVectorMatrix = StaticMatrix<std::array<T, deltas.size()>, N, M>;

template <typename T>
using DynamicVectorMatrix = DynamicMatrix<std::array<T, deltas.size()>>;

struct SimulationState {
    Fixed<> g;
    std::array<Fixed<>, rhoSize> rho;
    std::vector<std::vector<char>> field;

    size_t getFieldHeight() const { return field.size(); }
    size_t getFieldWidth() const {
        return field.size() > 0 ? field[0].size() : 0;
    }
};
