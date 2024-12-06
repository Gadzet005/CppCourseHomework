#include <algorithm>
#include <cli/type_parser.hpp>
#include <regex>

const regex pattern(
    R"((DOUBLE)|(FLOAT)|(FIXED)\((\d+), (\d+)\)|(FAST_FIXED)\((\d+), (\d+)\))",
    regex_constants::icase);

inline void toLower(std::string& str) {
    for (size_t i = 0; i < str.size(); ++i) {
        str[i] = tolower(str[i]);
    }
}

TypeId getTypeId(const string& typeName) {
    if (typeName == "double") {
        return TypeId::doubleType;
    } else if (typeName == "float") {
        return TypeId::floatType;
    } else if (typeName == "fixed") {
        return TypeId::fixedType;
    } else if (typeName == "fast_fixed") {
        return TypeId::fastFixedType;
    } else {
        throw invalid_argument("Invalid type name");
    }
}

Type parseType(string str) {
    smatch match;
    if (!regex_match(str, match, pattern)) {
        throw invalid_argument("Invalid type format");
    }

    string typeName = match[1].str();
    toLower(typeName);
    TypeId typeId = getTypeId(typeName);

    switch (typeId) {
        case TypeId::doubleType:
            return Type::doubleType();
        case TypeId::floatType:
            return Type::floatType();
        case TypeId::fixedType:
            return Type::fixedType(stoi(match[2]), stoi(match[3]));
        case TypeId::fastFixedType:
            return Type::fixedType(stoi(match[2]), stoi(match[3]));
        default:
            throw invalid_argument("Invalid type id");
    }
}