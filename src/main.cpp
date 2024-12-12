#include <cli/console_args.hpp>
#include <fstream>
#include <iostream>
#include <simulation/factory.hpp>
#include <simulation/save_load.hpp>
#include <types/fast_fixed.hpp>

using namespace std;

int main(int argc, char* argv[]) {
    auto args = parseConsoleArguments(argc, argv);
    auto [valid, error] = args.validate();
    if (!valid) {
        cout << error << endl;
        return 0;
    }

    unsigned tickCount = 0;
    FluidSimulationState state;

    if (!args.filePath.empty()) {
        ifstream in;
        in.open(args.filePath);
        if (!in.is_open()) {
            cout << "Error opening file: " << args.filePath << endl;
            return 0;
        }
        state = loadFluidSimulationStartState(in);
        cout << "Successfully loaded start state of simulation." << endl;
    } else {
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

    FactoryContext ctx = {
        state.getFieldHeight(), state.getFieldWidth(), args.pType,
        args.velocityType,      args.velocityFlowType, state};
    auto simulation = FluidSimulationFactory(ctx).create();

    cout << "\nPress anything to start." << endl;
    cout << "Press Ctrl+C to stop." << endl;
    getchar();

    simulation->print_field();
    while (true) {
        if (!simulation->step()) {
            continue;
        }

        cout << "Tick " << tickCount << ":\n";
        simulation->print_field();

        tickCount++;
        if (tickCount % args.saveRate == 0) {
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