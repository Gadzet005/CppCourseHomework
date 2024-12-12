#pragma once

#include <getopt.h>

#include <cli/type_parser.hpp>
#include <string>

struct ConsoleArgs {
    std::string filePath;
    Type pType;
    Type velocityType;
    Type velocityFlowType;
    std::string saveDir = "./save";
    size_t saveRate = 100;
    std::string saveFile;

    /// @brief Validate console args.
    /// @return result of validation and message, if not successful.
    std::pair<bool, std::string> validate();
};

ConsoleArgs parseConsoleArguments(int argc, char* argv[]);