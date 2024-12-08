#pragma once

#include <iostream>

using namespace std;

template <typename St, size_t K>
struct BaseFixed {
    using StoreType = St;

    static_assert(!is_same<StoreType, void>::value, "Invalid N parameter.");
    static_assert(K <= sizeof(StoreType) * 8, "Invalid K parameter.");

    constexpr BaseFixed() : v(0) {}
    constexpr BaseFixed(int v) : BaseFixed(static_cast<long long>(v)) {}
    constexpr BaseFixed(long long v) : v(v << K) {}
    constexpr BaseFixed(float f) : v(f * (StoreType(1) << K)) {}
    constexpr BaseFixed(double f) : v(f * (StoreType(1) << K)) {}

    template <typename OtherStoreType, size_t OtherK>
    constexpr BaseFixed(const BaseFixed<OtherStoreType, OtherK> &other) {
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

    static constexpr BaseFixed from_raw(StoreType x) {
        BaseFixed ret;
        ret.v = x;
        return ret;
    }

    auto operator<=>(const BaseFixed &) const = default;
    bool operator==(const BaseFixed &) const = default;

    friend BaseFixed operator+(BaseFixed a, BaseFixed b) {
        return BaseFixed::from_raw(a.v + b.v);
    }

    friend BaseFixed operator-(BaseFixed a, BaseFixed b) {
        return BaseFixed::from_raw(a.v - b.v);
    }

    friend BaseFixed operator*(BaseFixed a, BaseFixed b) {
        return BaseFixed::from_raw(((__int128_t)a.v * b.v) >> K);
    }

    friend BaseFixed operator/(BaseFixed a, BaseFixed b) {
        return BaseFixed::from_raw(((__int128_t)a.v << K) / b.v);
    }

    friend BaseFixed &operator+=(BaseFixed &a, BaseFixed b) {
        return a = a + b;
    }

    friend BaseFixed &operator-=(BaseFixed &a, BaseFixed b) {
        return a = a - b;
    }

    friend BaseFixed &operator*=(BaseFixed &a, BaseFixed b) {
        return a = a * b;
    }

    friend BaseFixed &operator/=(BaseFixed &a, BaseFixed b) {
        return a = a / b;
    }

    friend BaseFixed operator-(BaseFixed x) {
        return BaseFixed::from_raw(-x.v);
    }

    friend BaseFixed abs(BaseFixed x) {
        if (x.v < 0) {
            x.v = -x.v;
        }
        return x;
    }

    friend ostream &operator<<(ostream &out, const BaseFixed &x) {
        return out << x.v / (double)(StoreType(1) << K);
    }

    friend istream &operator>>(istream &in, BaseFixed &x) {
        double v;
        in >> v;
        x = BaseFixed(v);
        return in;
    }
};
