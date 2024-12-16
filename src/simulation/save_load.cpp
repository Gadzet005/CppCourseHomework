#include <cstddef>
#include <fstream>
#include <limits>
#include <simulation/save_load.hpp>

using namespace std;

FluidSimulationState loadFluidSimulationStartState(istream& in) {
    Fixed<> g;
    in >> g;

    size_t rhoCount;
    in >> rhoCount;

    array<Fixed<>, rhoSize> rho;
    for (size_t i = 0; i < rhoCount; ++i) {
        in.ignore(numeric_limits<streamsize>::max(), '\n');
        size_t idx = static_cast<size_t>(in.get());
        in >> rho[idx];
    }

    size_t height, width;
    in >> height >> width;
    DynamicMatrix<char> field(height, vector<char>(width));
    for (size_t x = 0; x < height; ++x) {
        in.ignore(numeric_limits<streamsize>::max(), '\n');
        for (size_t y = 0; y < width; ++y) {
            field[x][y] = in.get();
        }
    }

    FluidSimulationState state(std::move(field));
    state.g = g;
    state.rho = std::move(rho);

    return state;
}

pair<unsigned, FluidSimulationState> loadFluidSimulationState(istream& in) {
    unsigned tickCount;
    size_t height, width;

    in.read((char*)&tickCount, sizeof(tickCount));
    in.read((char*)&height, sizeof(height));
    in.read((char*)&width, sizeof(width));

    FluidSimulationState state = FluidSimulationState(height, width);

    int64_t raw;

    in.read((char*)&raw, sizeof(raw));
    state.g.v = raw;
    in.read((char*)&state.UT, sizeof(state.UT));

    for (size_t i = 0; i < rhoSize; ++i) {
        in.read((char*)&raw, sizeof(raw));
        state.rho[i].v = raw;
    }

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            in.read((char*)&state.field[i][j], sizeof(state.field[i][j]));

            in.read((char*)&raw, sizeof(raw));
            state.p[i][j].v = raw;

            in.read((char*)&state.dirs[i][j], sizeof(state.dirs[i][j]));
            in.read((char*)&state.last_use[i][j], sizeof(state.last_use[i][j]));

            for (auto& velocity : state.velocity[i][j]) {
                in.read((char*)&raw, sizeof(raw));
                velocity.v = raw;
            }
        }
    }

    return make_pair(tickCount, state);
}

void saveFluidSimulationState(ostream& out, const FluidSimulationState& state,
                              unsigned tickCount) {
    out.write((char*)&tickCount, sizeof(tickCount));

    size_t height = state.getFieldHeight();
    size_t width = state.getFieldWidth();
    out.write((char*)&height, sizeof(height));
    out.write((char*)&width, sizeof(width));

    int64_t raw;

    raw = int64_t(state.g.v);
    out.write((char*)&raw, sizeof(raw));
    out.write((char*)&state.UT, sizeof(state.UT));

    for (size_t i = 0; i < rhoSize; i++) {
        raw = int64_t(state.rho[i].v);
        out.write((char*)&raw, sizeof(raw));
    }

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            out.write(&state.field[i][j], sizeof(state.field[i][j]));

            raw = int64_t(state.p[i][j].v);
            out.write((char*)&raw, sizeof(raw));

            out.write((char*)&state.dirs[i][j], sizeof(state.dirs[i][j]));
            out.write((char*)&state.last_use[i][j],
                      sizeof(state.last_use[i][j]));

            for (const auto& velocity : state.velocity[i][j]) {
                raw = int64_t(velocity.v);
                out.write((char*)&raw, sizeof(raw));
            }
        }
    }
}