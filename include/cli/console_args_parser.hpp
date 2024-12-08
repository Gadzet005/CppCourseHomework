#pragma once

#include <getopt.h>

#include <cli/type_parser.hpp>
#include <string>

using namespace std;

struct SimulationTypes {
    Type pType = doubleType();
    Type velocityType = doubleType();
    Type velocityFlowType = doubleType();

    SimulationTypes() = default;
};

struct ConsoleArgs {
    string filePath;
    SimulationTypes types;

    ConsoleArgs() = default;
};

ConsoleArgs parseConsoleArguments(int argc, char* argv[]);
