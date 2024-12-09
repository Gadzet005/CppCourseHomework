#pragma once

class FluidSimulationInterface {
public:
    virtual ~FluidSimulationInterface() = default;
    virtual bool step() = 0;
    virtual void print_field() const = 0;
};
