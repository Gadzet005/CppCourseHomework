#pragma once

#include <simulation/base.hpp>

template <typename PType, typename VelocityType, typename VelocityFlowType>
class DynamicFluidSimulation
    : public BaseFluidSimulation<PType, VelocityType, VelocityFlowType> {
public:
    DynamicFluidSimulation(const SimulationState &state)
        : BaseFluidSimulation<PType, VelocityType, VelocityFlowType>(
              state.getFieldHeight(), state.getFieldWidth(), state.g,
              state.rho) {
        this->p.resize(this->height, std::vector<PType>(this->width));
        this->old_p.resize(this->height, std::vector<PType>(this->width));
        this->dirs.resize(this->height, std::vector<int>(this->width, 0));
        this->last_use.resize(this->height, std::vector<int>(this->width, 0));
        this->velocity.v.resize(
            this->height,
            std::vector<std::array<VelocityType, deltas.size()>>(
                this->width, std::array<VelocityType, deltas.size()>()));
        this->velocity_flow.v.resize(
            this->height,
            std::vector<std::array<VelocityFlowType, deltas.size()>>(
                this->width, std::array<VelocityType, deltas.size()>()));

        this->field = state.field;

        for (size_t x = 0; x < this->height; ++x) {
            for (size_t y = 0; y < this->width; ++y) {
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
