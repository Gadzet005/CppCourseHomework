#include <cli/type_parser.hpp>
#include <regex>

const regex patterns[] = {
    regex(R"((DOUBLE))", regex_constants::icase),
    regex(R"((FLOAT))", regex_constants::icase),
    regex(R"((FAST_FIXED)\((\d+),\s*(\d+)\))", regex_constants::icase),
    regex(R"((FIXED)\((\d+),\s*(\d+)\))", regex_constants::icase),
};

smatch getMatch(const string& str) {
    smatch match;
    for (const auto& pattern : patterns) {
        if (regex_match(str, match, pattern)) {
            return match;
        }
    }
    throw invalid_argument("Invalid type.");
}

inline void toLower(std::string& str) {
    for (size_t i = 0; i < str.size(); ++i) {
        str[i] = tolower(str[i]);
    }
}

Type parseType(string str) {
    smatch match = getMatch(str);

    string typeName = match[1].str();
    toLower(typeName);

    if (typeName == "float") {
        return floatType();
    } else if (typeName == "double") {
        return doubleType();
    } else if (typeName == "fixed") {
        return fixedType(stoi(match[2]), stoi(match[3]));
    } else if (typeName == "fast_fixed") {
        return fastFixedType(stoi(match[2]), stoi(match[3]));
    }
    throw invalid_argument("Unknown type name.");
}