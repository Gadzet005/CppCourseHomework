#pragma once

#include <cstdint>
#include <types/base_fixed.hpp>

namespace FixedInternal {

template <size_t N>
struct Store {
    using Type = void;
};

template <>
struct Store<8> {
    using Type = int8_t;
};

template <>
struct Store<16> {
    using Type = int16_t;
};

template <>
struct Store<32> {
    using Type = int32_t;
};

template <>
struct Store<64> {
    using Type = int64_t;
};

}  // namespace FixedInternal

template <size_t N = 32, size_t K = 16>
using Fixed = BaseFixed<typename FixedInternal::Store<N>::Type, K>;
