#pragma once

#include <array>
#include <functional>
#include <memory>
#include <simulation/dynamic.hpp>
#include <simulation/interface.hpp>
#include <simulation/static.hpp>
#include <tuple>
#include <types/fast_fixed.hpp>
#include <types/fixed.hpp>
#include <types/type.hpp>

struct FactoryContext {
    size_t height, width;
    Type pType, velocityType, velocityFlowType;
    SimulationState initialState;
};

namespace internal {

using Factory = std::function<std::unique_ptr<FluidSimulationInterface>(
    const FactoryContext&)>;

namespace StaticFieldFactory {
template <typename PType, typename VelocityType, typename VelocityFlowType,
          size_t Height, size_t Width>
std::unique_ptr<FluidSimulationInterface> create(const FactoryContext& ctx) {
    std::cout << "Used static field(" << Height << ", " << Width << ")"
              << std::endl;
    return std::make_unique<StaticFluidSimulation<
        Height, Width, PType, VelocityType, VelocityFlowType>>(
        ctx.initialState);
}
}  // namespace StaticFieldFactory

namespace SizeFactory {
template <typename PType, typename VelocityType, typename VelocityFlowType>
std::unique_ptr<FluidSimulationInterface> create(const FactoryContext& ctx) {
#define S(height, width)                                                  \
    std::make_tuple(                                                      \
        height, width,                                                    \
        StaticFieldFactory::create<PType, VelocityType, VelocityFlowType, \
                                   height, width>)

    static const std::tuple<size_t, size_t, Factory> factories[] = {SIZES};

#undef S

    for (const auto& [height, width, factory] : factories) {
        if (ctx.height == height && ctx.width == width) {
            return factory(ctx);
        }
    }

    std::cout << "Used dynamic field(" << ctx.height << ", " << ctx.width << ")"
              << std::endl;
    return std::make_unique<
        DynamicFluidSimulation<PType, VelocityType, VelocityFlowType>>(
        ctx.initialState);
}
}  // namespace SizeFactory

namespace VelocityFlowTypeFactory {
template <typename PType, typename VelocityType>
std::unique_ptr<FluidSimulationInterface> create(const FactoryContext& ctx) {
#define DOUBLE \
    { doubleType(), SizeFactory::create<PType, VelocityType, double> }
#define FLOAT \
    { floatType(), SizeFactory::create<PType, VelocityType, float> }
#define FIXED(n, k) \
    { fixedType(n, k), SizeFactory::create<PType, VelocityType, Fixed<n, k>> }
#define FAST_FIXED(n, k)                                              \
    {                                                                 \
        fastFixedType(n, k),                                          \
            SizeFactory::create<PType, VelocityType, FastFixed<n, k>> \
    }

    static const std::pair<Type, Factory> factories[] = {TYPES};

#undef DOUBLE
#undef FLOAT
#undef FIXED
#undef FAST_FIXED

    for (const auto& [type, factory] : factories) {
        if (ctx.velocityFlowType == type) {
            std::cout << "VFlowType used: " << toString(ctx.velocityFlowType)
                      << std::endl;
            return factory(ctx);
        }
    }
    throw std::invalid_argument("Unsupported velocity flow type.");
}
}  // namespace VelocityFlowTypeFactory

namespace VelocityTypeFactory {
template <typename PType>
std::unique_ptr<FluidSimulationInterface> create(const FactoryContext& ctx) {
#define DOUBLE \
    { doubleType(), VelocityFlowTypeFactory::create<PType, double> }
#define FLOAT \
    { floatType(), VelocityFlowTypeFactory::create<PType, float> }
#define FIXED(n, k) \
    { fixedType(n, k), VelocityFlowTypeFactory::create<PType, Fixed<n, k>> }
#define FAST_FIXED(n, k)                                            \
    {                                                               \
        fastFixedType(n, k),                                        \
            VelocityFlowTypeFactory::create<PType, FastFixed<n, k>> \
    }

    static const std::pair<Type, Factory> factories[] = {TYPES};

#undef DOUBLE
#undef FLOAT
#undef FIXED
#undef FAST_FIXED

    for (const auto& [type, factory] : factories) {
        if (ctx.velocityType == type) {
            std::cout << "VType used: " << toString(ctx.velocityType)
                      << std::endl;
            return factory(ctx);
        }
    }
    throw std::invalid_argument("Unsupported velocity type.");
}
}  // namespace VelocityTypeFactory

namespace PTypeFactory {
std::unique_ptr<FluidSimulationInterface> create(const FactoryContext& ctx) {
#define DOUBLE \
    { doubleType(), VelocityTypeFactory::create<double> }
#define FLOAT \
    { floatType(), VelocityTypeFactory::create<float> }
#define FIXED(n, k) \
    { fixedType(n, k), VelocityTypeFactory::create<Fixed<n, k>> }
#define FAST_FIXED(n, k) \
    { fastFixedType(n, k), VelocityTypeFactory::create<FastFixed<n, k>> }

    static const std::pair<Type, Factory> factories[] = {TYPES};

#undef DOUBLE
#undef FLOAT
#undef FIXED
#undef FAST_FIXED

    for (const auto& [type, factory] : factories) {
        if (ctx.pType == type) {
            std::cout << "PType used: " << toString(ctx.pType) << std::endl;
            return factory(ctx);
        }
    }
    throw std::invalid_argument("Unsupported p type.");
}
}  // namespace PTypeFactory

}  // namespace internal

class FluidSimulationFactory {
public:
    FluidSimulationFactory(const FactoryContext& ctx) : ctx(ctx) {}

    std::unique_ptr<FluidSimulationInterface> create() const {
        return internal::PTypeFactory::create(ctx);
    }

private:
    FactoryContext ctx;
};