#include <bits/stdc++.h>

#include <simulation/fluid_simulation.hpp>
#include <types/fixed.hpp>

using namespace std;

constexpr size_t N = 36, M = 84;
constexpr size_t T = 1'000'000;

int main() {
    auto simulation = load_from_file<N, M>("input.txt");

    for (size_t i = 0; i < T; ++i) {
        bool res = simulation.step();
        if (res) {
            cout << "Tick " << i << ":\n";
            simulation.print_field();
        }
    }
}