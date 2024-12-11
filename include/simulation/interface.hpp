#pragma once

#include <iostream>

class FluidSimulationInterface {
public:
    virtual ~FluidSimulationInterface() = default;
    virtual bool step() = 0;
    virtual void print_field(std::ostream& out = std::cout) const = 0;
};
