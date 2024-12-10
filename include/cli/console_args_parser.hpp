#pragma once

#include <getopt.h>

#include <cli/type_parser.hpp>
#include <string>

struct ConsoleArgs {
    std::string filePath;
    Type pType;
    Type velocityType;
    Type velocityFlowType;
};

ConsoleArgs parseConsoleArguments(int argc, char* argv[]);
