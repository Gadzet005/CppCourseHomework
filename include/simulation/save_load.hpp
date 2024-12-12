#pragma once

#include <iostream>
#include <simulation/common.hpp>

/// @brief Load start state of fluid simulation.
/// @return State of fluid simulation.
FluidSimulationState loadFluidSimulationStartState(std::istream& in);

/// @brief Load any state of fluid simulation.
/// @return Tick count and state.
std::pair<unsigned, FluidSimulationState> loadFluidSimulationState(
    std::istream& in);

void saveFluidSimulationState(std::ostream& out,
                              const FluidSimulationState& state,
                              unsigned tickCount);
