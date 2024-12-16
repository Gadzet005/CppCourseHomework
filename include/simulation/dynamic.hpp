#pragma once

#include <simulation/base.hpp>

/// @brief Fluid simulation with dynamic field.
template <typename PType, typename VelocityType, typename VelocityFlowType>
class DynamicFluidSimulation
    : public BaseFluidSimulation<PType, VelocityType, VelocityFlowType> {
public:
    DynamicFluidSimulation(const FluidSimulationState &state)
        : BaseFluidSimulation<PType, VelocityType, VelocityFlowType>(
              state.getFieldHeight(), state.getFieldWidth(), state.g,
              state.rho) {
        // copy hell

        this->UT = state.UT;

        this->field.resize(this->height, std::vector<char>(this->width));
        this->p.resize(this->height, std::vector<PType>(this->width));
        this->dirs.resize(this->height, std::vector<int>(this->width, 0));
        this->last_use.resize(this->height, std::vector<int>(this->width, 0));
        this->velocity.v.resize(
            this->height,
            std::vector<std::array<VelocityType, deltas.size()>>(
                this->width, std::array<VelocityType, deltas.size()>()));
        this->velocity_flow.v.resize(
            this->height,
            std::vector<std::array<VelocityFlowType, deltas.size()>>(
                this->width, std::array<VelocityFlowType, deltas.size()>()));
        this->old_p = this->p;

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
