#pragma once

#include <cli/type_parser.hpp>
#include <string>

struct ConsoleArgs {
    std::string filePath;
    Type pType;
    Type velocityType;
    Type velocityFlowType;
    std::string saveDir = "./save";
    std::string saveFile;
    unsigned long saveRate = 100;
    unsigned long maxIterations = 10000;
    bool quiet = false;

    /// @brief Validate console args.
    /// @return result of validation and message, if not successful.
    std::pair<bool, std::string> validate();
};

ConsoleArgs parseConsoleArguments(int argc, char* argv[]);