#pragma once

#include <iostream>

/// @brief Real number.
/// @tparam St store type (int, long long, ...)
/// @tparam _K bit quantity for fractional part.
template <typename St, size_t _K>
struct BaseFixed {
    using StoreType = St;
    static constexpr size_t N = sizeof(StoreType) * 8;
    static constexpr size_t K = _K;

    static_assert(!std::is_same<StoreType, void>::value,
                  "Invalid N parameter.");
    static_assert(K <= N, "Invalid K parameter.");

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

    static constexpr BaseFixed fromRaw(StoreType x) {
        BaseFixed ret;
        ret.v = x;
        return ret;
    }

    auto operator<=>(const BaseFixed &) const = default;
    bool operator==(const BaseFixed &) const = default;

    friend BaseFixed operator+(BaseFixed a, BaseFixed b) {
        return BaseFixed::fromRaw(a.v + b.v);
    }

    friend BaseFixed operator-(BaseFixed a, BaseFixed b) {
        return BaseFixed::fromRaw(a.v - b.v);
    }

    // TODO: remove __int128
    friend BaseFixed operator*(BaseFixed a, BaseFixed b) {
        return BaseFixed::fromRaw(((__int128_t)a.v * b.v) >> K);
    }

    // TODO: remove __int128
    friend BaseFixed operator/(BaseFixed a, BaseFixed b) {
        return BaseFixed::fromRaw(((__int128_t)a.v << K) / b.v);
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

    friend BaseFixed operator-(BaseFixed x) { return BaseFixed::fromRaw(-x.v); }

    friend BaseFixed abs(BaseFixed x) {
        if (x.v < 0) {
            x.v = -x.v;
        }
        return x;
    }

    friend std::ostream &operator<<(std::ostream &out, const BaseFixed &x) {
        return out << double(x);
    }

    friend std::istream &operator>>(std::istream &in, BaseFixed &x) {
        double v;
        in >> v;
        x = BaseFixed(v);
        return in;
    }

    explicit operator int() const { return v >> K; }
    explicit operator int64_t() const { return int64_t(v >> K); }
    explicit operator float() const { return v / (float)(StoreType(1) << K); }
    explicit operator double() const { return v / (double)(StoreType(1) << K); }
};
