#pragma once

#include <cstddef>
#include <string>

using namespace std;

enum class TypeId { doubleType, floatType, fixedType, fastFixedType };

struct Type {
    TypeId typeId;
    size_t n, k;

    constexpr Type() : typeId(TypeId::doubleType), n(0), k(0) {}
    constexpr Type(TypeId id) : typeId(id) {}
    constexpr Type(TypeId id, size_t n, size_t k) : typeId(id), n(n), k(k) {}

    bool operator==(const Type&) const = default;
};

constexpr Type doubleType() { return Type(TypeId::doubleType); }

constexpr Type floatType() { return Type(TypeId::floatType); }

constexpr Type fixedType(size_t n, size_t k) {
    return Type(TypeId::fixedType, n, k);
}

constexpr Type fastFixedType(size_t n, size_t k) {
    return Type(TypeId::fastFixedType, n, k);
}

string toString(const Type& type);