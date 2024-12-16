#pragma once

#include <iostream>
#include <simulation/common.hpp>

/// @brief Load start state of fluid simulation from text file.
/// @return State of fluid simulation.
FluidSimulationState loadFluidSimulationStartState(std::istream& in);

/// @brief Load any state of fluid simulation from bin file.
/// @return Tick count and state.
std::pair<unsigned, FluidSimulationState> loadFluidSimulationState(
    std::istream& in);

/// @brief Save any state of fluid simulation to bin file.
void saveFluidSimulationState(std::ostream& out,
                              const FluidSimulationState& state,
                              unsigned tickCount);
