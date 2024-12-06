#pragma once

#include <cstddef>
#include <string>

using namespace std;

enum class TypeId { doubleType, floatType, fixedType, fastFixedType };

struct Type {
    TypeId typeId;
    size_t n, k;

    constexpr Type(TypeId id) : typeId(id) {}
    constexpr Type(TypeId id, size_t n, size_t k) : typeId(id), n(n), k(k) {}

    static constexpr Type doubleType() { return Type(TypeId::doubleType); }

    static constexpr Type floatType() { return Type(TypeId::floatType); }

    static constexpr Type fixedType(size_t n, size_t k) {
        return Type(TypeId::fixedType, n, k);
    }

    static constexpr Type FastFixed(size_t n, size_t k) {
        return Type(TypeId::fixedType, n, k);
    }
};

Type parseType(string str);
