#include <types/type.hpp>

string toString(const Type& type) {
    switch (type.typeId) {
        case TypeId::doubleType:
            return "double";
        case TypeId::floatType:
            return "float";
        case TypeId::fixedType:
            return "fixed(" + to_string(type.n) + "," + to_string(type.k) + ")";
        case TypeId::fastFixedType:
            return "fast_fixed(" + to_string(type.n) + "," + to_string(type.k) +
                   ")";
        default:
            return "Unknown";
    }
}