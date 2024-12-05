#pragma once

#include <cstdint>
#include <iostream>

using namespace std;

struct Fixed {
    constexpr Fixed(int v) : v(v << 16) {}
    constexpr Fixed(float f) : v(f * (1 << 16)) {}
    constexpr Fixed(double f) : v(f * (1 << 16)) {}
    constexpr Fixed() : v(0) {}

    static constexpr Fixed from_raw(int32_t x) {
        Fixed ret;
        ret.v = x;
        return ret;
    }

    int32_t v;

    auto operator<=>(const Fixed &) const = default;
    bool operator==(const Fixed &) const = default;
};

Fixed operator+(Fixed a, Fixed b) { return Fixed::from_raw(a.v + b.v); }

Fixed operator-(Fixed a, Fixed b) { return Fixed::from_raw(a.v - b.v); }

Fixed operator*(Fixed a, Fixed b) {
    return Fixed::from_raw(((int64_t)a.v * b.v) >> 16);
}

Fixed operator/(Fixed a, Fixed b) {
    return Fixed::from_raw(((int64_t)a.v << 16) / b.v);
}

Fixed &operator+=(Fixed &a, Fixed b) { return a = a + b; }

Fixed &operator-=(Fixed &a, Fixed b) { return a = a - b; }

Fixed &operator*=(Fixed &a, Fixed b) { return a = a * b; }

Fixed &operator/=(Fixed &a, Fixed b) { return a = a / b; }

Fixed operator-(Fixed x) { return Fixed::from_raw(-x.v); }

Fixed abs(Fixed x) {
    if (x.v < 0) {
        x.v = -x.v;
    }
    return x;
}

ostream &operator<<(ostream &out, const Fixed &x) {
    return out << x.v / (double)(1 << 16);
}

istream &operator>>(istream &in, Fixed &x) {
    double v;
    in >> v;
    x = Fixed(v);
    return in;
}