#pragma once

#include <cstdint>
#include <iostream>
#include <type_traits>

using namespace std;

namespace internal {

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

}  // namespace internal

template <size_t N = 32, size_t K = 16>
struct Fixed {
    using StoreType = internal::Store<N>::Type;

    static_assert(!is_same<StoreType, void>::value, "Invalid N parameter.");
    static_assert(K <= N, "Invalid K parameter.");

    constexpr Fixed() : v(0) {}
    constexpr Fixed(int v) : Fixed(static_cast<long long>(v)) {}
    constexpr Fixed(long long v) : v(v << K) {}
    constexpr Fixed(float f) : v(f * (1 << K)) {}
    constexpr Fixed(double f) : v(f * (1 << K)) {}

    template <size_t OtherN, size_t OtherK>
    constexpr Fixed(const Fixed<OtherN, OtherK> &other) {
        auto otherV = other.v;
        if (OtherK > K) {
            otherV >>= (OtherK - K);
        }

        v = static_cast<StoreType>(otherV);
        if (K > OtherK) {
            v <<= (K - OtherK);
        }
    }

    StoreType v;

    static constexpr Fixed from_raw(StoreType x) {
        Fixed ret;
        ret.v = x;
        return ret;
    }

    auto operator<=>(const Fixed &) const = default;
    bool operator==(const Fixed &) const = default;

    friend Fixed operator+(Fixed a, Fixed b) {
        return Fixed::from_raw(a.v + b.v);
    }

    friend Fixed operator-(Fixed a, Fixed b) {
        return Fixed::from_raw(a.v - b.v);
    }

    friend Fixed operator*(Fixed a, Fixed b) {
        return Fixed::from_raw(((__int128_t)a.v * b.v) >> K);
    }

    friend Fixed operator/(Fixed a, Fixed b) {
        return Fixed::from_raw(((__int128_t)a.v << K) / b.v);
    }

    friend Fixed &operator+=(Fixed &a, Fixed b) { return a = a + b; }

    friend Fixed &operator-=(Fixed &a, Fixed b) { return a = a - b; }

    friend Fixed &operator*=(Fixed &a, Fixed b) { return a = a * b; }

    friend Fixed &operator/=(Fixed &a, Fixed b) { return a = a / b; }

    friend Fixed operator-(Fixed x) { return Fixed::from_raw(-x.v); }

    friend Fixed abs(Fixed x) {
        if (x.v < 0) {
            x.v = -x.v;
        }
        return x;
    }

    friend ostream &operator<<(ostream &out, const Fixed &x) {
        return out << x.v / (double)(1 << K);
    }

    friend istream &operator>>(istream &in, Fixed &x) {
        double v;
        in >> v;
        x = Fixed(v);
        return in;
    }
};
