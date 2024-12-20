#pragma once

#include <cli/console_args.hpp>
#include <simulation/common.hpp>

FluidSimulationState loadStateByArgs(const ConsoleArgs& args);
void saveStateByArgs(const ConsoleArgs& args,
                     const FluidSimulationState& state);