#include <cli/console_args.hpp>
#include <fstream>
#include <iostream>
#include <simulation/factory.hpp>
#include <simulation/save_load.hpp>
#include <types/fast_fixed.hpp>

using namespace std;

int main(int argc, char* argv[]) {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    auto args = parseConsoleArguments(argc, argv);
    auto [valid, error] = args.validate();
    if (!valid) {
        cout << error << endl;
        return 0;
    }

    unsigned tickCount = 0;
    FluidSimulationState state;

    if (!args.filePath.empty()) {
        // Load initial state of simulation from text file.
        ifstream in;
        in.open(args.filePath);
        if (!in.is_open()) {
            cout << "Error opening file: " << args.filePath << endl;
            return 0;
        }
        state = loadFluidSimulationStartState(in);
        cout << "Successfully loaded start state of simulation." << endl;
    } else {
        // Load saved state of simulation from bin file.
        ifstream in;
        in.open(args.saveFile, ios::binary);
        if (!in.is_open()) {
            cout << "Error opening file: " << args.filePath << endl;
            return 0;
        }
        auto [_tickCount, _state] = loadFluidSimulationState(in);
        tickCount = _tickCount;
        state = std::move(_state);
        cout << "Successfully loaded state of simulation, tickCount = "
             << tickCount << "." << endl;
    }

    // Create simulation.
    FactoryContext ctx = {
        state.getFieldHeight(), state.getFieldWidth(), args.pType,
        args.velocityType,      args.velocityFlowType, state};
    auto simulation = FluidSimulationFactory(ctx).create();

    cout << "\nPress anything to start." << endl;
    cout << "Press Ctrl+C to stop." << endl;
    getchar();

    if (!args.quiet) {
        simulation->print_field();
    }

    for (unsigned long i = 0; i < args.maxIterations; i++) {
        if (!simulation->step()) {
            continue;
        }

        cout << "Tick " << tickCount << endl;
        if (!args.quiet) {
            simulation->print_field();
        }

        tickCount++;
        if (tickCount % args.saveRate == 0) {
            // Save state of simulation to bin file.
            string saveFilePath = args.saveDir + "/" + to_string(tickCount);
            ofstream out;
            out.open(saveFilePath, ios::binary);
            if (!out.is_open()) {
                cout << "Save error occurred." << endl;
                return 0;
            }
            saveFluidSimulationState(out, simulation->getState(), tickCount);
            cout << "Saved state to file: " << saveFilePath << "." << endl;
        }
    }

    return 0;
}