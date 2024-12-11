#pragma once

#include <simulation/base.hpp>

template <typename PType, typename VelocityType, typename VelocityFlowType,
          size_t Height, size_t Width>
class StaticFluidSimulation
    : public BaseFluidSimulation<PType, VelocityType, VelocityFlowType, Height,
                                 Width> {
public:
    StaticFluidSimulation(const SimulationState &state)
        : BaseFluidSimulation<PType, VelocityType, VelocityFlowType, Height,
                              Width>(Height, Width, state.g, state.rho) {
        for (size_t x = 0; x < this->height; ++x) {
            for (size_t y = 0; y < this->width; ++y) {
                this->field[x][y] = state.field[x][y];
                if (state.field[x][y] != '#') {
                    for (auto [dx, dy] : deltas) {
                        this->dirs[x][y] +=
                            (state.field[x + dx][y + dy] != '#');
                    }
                }
            }
        }
    }
};
