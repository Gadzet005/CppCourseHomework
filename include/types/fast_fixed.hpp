#pragma once

#include <cstdint>
#include <types/base_fixed.hpp>

using namespace std;

namespace FastFixedInternal {

template <size_t N>
struct Store {
    using Type = void;
};

template <>
struct Store<8> {
    using Type = int_fast8_t;
};

template <>
struct Store<16> {
    using Type = int_fast16_t;
};

template <>
struct Store<32> {
    using Type = int_fast32_t;
};

template <>
struct Store<64> {
    using Type = int_fast64_t;
};

template <size_t N>
struct FastStore {
    static constexpr size_t getSize() {
        if (N > 64) {
            return N;
        }
        if (N > 32) {
            return 64;
        }
        if (N > 16) {
            return 32;
        }
        if (N > 8) {
            return 16;
        }
        return 8;
    }
    using Type = typename Store<getSize()>::Type;
};

}  // namespace FastFixedInternal

template <size_t N = 32, size_t K = 16>
using FastFixed = BaseFixed<typename FastFixedInternal::FastStore<N>::Type, K>;
