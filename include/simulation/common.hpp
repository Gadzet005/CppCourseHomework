#pragma once

#include <array>
#include <types/fixed.hpp>
#include <vector>

constexpr unsigned rhoSize = 256;
constexpr std::array<std::pair<int, int>, 4> deltas{
    {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

constexpr size_t getDeltaIndex(int dx, int dy) {
    for (size_t i = 0; i < deltas.size(); ++i) {
        if (deltas[i].first == dx && deltas[i].second == dy) {
            return i;
        }
    }
    return deltas.size();
}

template <typename T, size_t N, size_t M>
using StaticMatrix = T[N][M];

template <typename T>
using DynamicMatrix = std::vector<std::vector<T>>;

template <typename T, size_t N, size_t M>
using StaticVectorMatrix = StaticMatrix<std::array<T, deltas.size()>, N, M>;

template <typename T>
using DynamicVectorMatrix = DynamicMatrix<std::array<T, deltas.size()>>;

/// @brief State of fluid simulation.
struct FluidSimulationState {
    Fixed<> g;
    std::array<Fixed<>, rhoSize> rho;
    DynamicMatrix<char> field;
    DynamicMatrix<Fixed<>> p;
    DynamicVectorMatrix<Fixed<>> velocity;
    DynamicMatrix<int> lastUse;
    DynamicMatrix<int> dirs;
    int UT = 0;
    unsigned tickCount = 0;

    explicit FluidSimulationState() = default;

    explicit FluidSimulationState(size_t height, size_t width) {
        field.resize(height, std::vector<char>(width));
        p.resize(height, std::vector<Fixed<>>(width));
        dirs.resize(height, std::vector<int>(width, 0));
        lastUse.resize(height, std::vector<int>(width, 0));
        velocity.resize(height,
                        std::vector<std::array<Fixed<>, deltas.size()>>(
                            width, std::array<Fixed<>, deltas.size()>()));
    }

    explicit FluidSimulationState(DynamicMatrix<char>&& initialField)
        : FluidSimulationState(initialField.size(), initialField.size() > 0
                                                        ? initialField[0].size()
                                                        : 0) {
        field = std::move(initialField);
        for (size_t x = 0; x < getFieldHeight(); ++x) {
            for (size_t y = 0; y < getFieldWidth(); ++y) {
                if (field[x][y] != '#') {
                    for (auto [dx, dy] : deltas) {
                        dirs[x][y] += (field[x + dx][y + dy] != '#');
                    }
                }
            }
        }
    }

    size_t getFieldHeight() const { return field.size(); }
    size_t getFieldWidth() const {
        return field.size() > 0 ? field[0].size() : 0;
    }
};
