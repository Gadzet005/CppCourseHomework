#pragma once

#include <getopt.h>

#include <cli/type_parser.hpp>
#include <string>

using namespace std;

struct ConsoleArgs {
    string filePath;
    Type pType;
    Type velocityType;
    Type velocityFlowType;

    ConsoleArgs() = default;
};

ConsoleArgs parseConsoleArguments(int argc, char* argv[]);
