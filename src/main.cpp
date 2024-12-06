#include <bits/stdc++.h>

#include <cli/console_args_parser.hpp>
#include <iostream>
#include <simulation/fluid_simulation.hpp>
#include <simulation/macro.hpp>
#include <types/fixed.hpp>
#include <types/macro.hpp>

using namespace std;

constexpr size_t N = 36, M = 84;
constexpr size_t T = 1'000'000;

int main(int argc, char* argv[]) {
    TYPES;
    SIZES;

    auto args = parseConsoleArguments(argc, argv);

    if (args.filePath.empty()) {
        cout << "Error: Input file path is not provided" << endl;
        return 0;
    }

    auto simulation =
        load_from_file<N, M, Fixed<64, 32>, Fixed<64, 32>, Fixed<64, 32>>(
            args.filePath);

    for (size_t i = 0; i < T; ++i) {
        bool res = simulation.step();
        if (res) {
            cout << "Tick " << i << ":\n";
            simulation.print_field();
        }
    }

    return 0;
}