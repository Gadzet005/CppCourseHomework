#pragma once

#include <simulation/base.hpp>

/// @brief Fluid simulation with static field.
template <typename PType, typename VelocityType, typename VelocityFlowType,
          size_t Height, size_t Width>
class StaticFluidSimulation
    : public BaseFluidSimulation<PType, VelocityType, VelocityFlowType, Height,
                                 Width> {
public:
    StaticFluidSimulation(const FluidSimulationState &state)
        : BaseFluidSimulation<PType, VelocityType, VelocityFlowType, Height,
                              Width>(Height, Width, state.g, state.rho) {
        // copy hell

        this->UT = state.UT;
        for (size_t x = 0; x < this->height; ++x) {
            for (size_t y = 0; y < this->width; ++y) {
                this->field[x][y] = state.field[x][y];
                this->p[x][y] = PType(state.p[x][y]);
                this->dirs[x][y] = state.dirs[x][y];
                this->last_use[x][y] = state.last_use[x][y];

                for (size_t k = 0; k < deltas.size(); k++) {
                    this->velocity.v[x][y][k] =
                        VelocityType(state.velocity[x][y][k]);
                }
            }
        }
    }
};
