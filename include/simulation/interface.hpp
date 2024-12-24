#pragma once

#include <iostream>
#include <simulation/common.hpp>

/// @brief Simple fluid simualtion interface.
class FluidSimulationInterface {
public:
    virtual ~FluidSimulationInterface() = default;

    /// @brief step of simulation.
    /// @return is simulation field change state?
    virtual bool step() = 0;
    /// @brief get number of steps, that somehow changes simulation field
    virtual unsigned getTickCount() const = 0;
    virtual void printField(std::ostream& out = std::cout) const = 0;
    virtual FluidSimulationState getState() const = 0;
};
