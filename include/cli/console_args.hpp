#pragma once

#include <cli/type_parser.hpp>
#include <string>

struct ConsoleArgs {
    // file with simulation start state description
    std::string inputFile;

    Type pType;
    Type velocityType;
    Type velocityFlowType;

    // dir for saves
    std::string saveDir = "./save";
    // file to load save
    std::string saveFile;
    // save rate (in ticks)
    unsigned saveRate = 100;

    // max simulation iterations (iteration != tick)
    unsigned maxIterations = 10000;
    // don't print simulation field to stdout each tick
    bool quiet = false;

    // number of threads for parallel computation
    unsigned threads = 1;

    /// @brief Validate console args.
    /// @return result of validation and message, if not successful.
    std::pair<bool, std::string> validate();
};

ConsoleArgs parseConsoleArguments(int argc, char* argv[]);