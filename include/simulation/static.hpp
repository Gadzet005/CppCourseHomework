#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <ranges>
#include <simulation/common.hpp>
#include <simulation/interface.hpp>
#include <types/fixed.hpp>

using namespace std;

template <typename T, size_t N, size_t M>
using Matrix = array<array<T, M>, N>;

template <size_t H, size_t W, typename PType, typename VelocityType,
          typename VelocityFlowType>
class StaticFluidSimulation : virtual FluidSimulationInterface {
public:
    static constexpr size_t Height = H;
    static constexpr size_t Width = W;

    StaticFluidSimulation(const SimulationDescription &description)
        : rho(description.rho), g(description.g) {
        for (size_t x = 0; x < Height; ++x) {
            for (size_t y = 0; y < Width; ++y) {
                field[x][y] = description.field[x][y];

                if (description.field[x][y] != '#') {
                    for (auto [dx, dy] : deltas) {
                        dirs[x][y] +=
                            (description.field[x + dx][y + dy] != '#');
                    }
                }
            }
        }
    }

    bool step() override {
        Fixed<> total_delta_p = 0;
        // Apply external forces
        for (size_t x = 0; x < Height; ++x) {
            for (size_t y = 0; y < Width; ++y) {
                if (field[x][y] == '#') continue;
                if (field[x + 1][y] != '#') velocity.add(x, y, 1, 0, g);
            }
        }

        // Apply forces from p
        old_p = p;
        for (size_t x = 0; x < Height; ++x) {
            for (size_t y = 0; y < Width; ++y) {
                if (field[x][y] == '#') continue;
                for (auto [dx, dy] : deltas) {
                    int nx = x + dx, ny = y + dy;
                    if (field[nx][ny] != '#' && old_p[nx][ny] < old_p[x][y]) {
                        auto delta_p = old_p[x][y] - old_p[nx][ny];
                        auto force = delta_p;
                        auto &contr = velocity.get(nx, ny, -dx, -dy);
                        if (contr * VelocityType(rho[(int)field[nx][ny]]) >=
                            force) {
                            contr -= force / PType(rho[(int)field[nx][ny]]);
                            continue;
                        }
                        force -= contr * VelocityType(rho[(int)field[nx][ny]]);
                        contr = 0;
                        velocity.add(x, y, dx, dy,
                                     force / PType(rho[(int)field[x][y]]));
                        p[x][y] -= force / dirs[x][y];
                        total_delta_p -= force / dirs[x][y];
                    }
                }
            }
        }

        // Make flow from velocities
        velocity_flow = {};
        bool prop = false;
        do {
            UT += 2;
            prop = 0;
            for (size_t x = 0; x < Height; ++x) {
                for (size_t y = 0; y < Width; ++y) {
                    if (field[x][y] != '#' && last_use[x][y] != UT) {
                        auto [t, local_prop, _] = propagate_flow(x, y, 1);
                        if (t > 0) {
                            prop = 1;
                        }
                    }
                }
            }
        } while (prop);

        // Recalculate p with kinetic energy
        for (size_t x = 0; x < Height; ++x) {
            for (size_t y = 0; y < Width; ++y) {
                if (field[x][y] == '#') continue;
                for (auto [dx, dy] : deltas) {
                    auto old_v = velocity.get(x, y, dx, dy);
                    auto new_v = velocity_flow.get(x, y, dx, dy);
                    if (old_v > 0) {
                        assert(new_v <= old_v);
                        velocity.get(x, y, dx, dy) = new_v;
                        auto force = (old_v - VelocityType(new_v)) *
                                     VelocityType(rho[(int)field[x][y]]);
                        if (field[x][y] == '.') force *= Fixed(0.8);
                        if (field[x + dx][y + dy] == '#') {
                            p[x][y] += force / dirs[x][y];
                            total_delta_p += force / dirs[x][y];
                        } else {
                            p[x + dx][y + dy] += force / dirs[x + dx][y + dy];
                            total_delta_p += force / dirs[x + dx][y + dy];
                        }
                    }
                }
            }
        }

        UT += 2;
        prop = false;
        for (size_t x = 0; x < Height; ++x) {
            for (size_t y = 0; y < Width; ++y) {
                if (field[x][y] != '#' && last_use[x][y] != UT) {
                    if (random01() < move_prob(x, y)) {
                        prop = true;
                        propagate_move(x, y, true);
                    } else {
                        propagate_stop(x, y, true);
                    }
                }
            }
        }

        return prop;
    }

    void print_field() const override {
        for (size_t x = 0; x < Height; ++x) {
            for (size_t y = 0; y < Width; ++y) {
                cout << field[x][y];
            }
            cout << '\n';
        }
    }

private:
    template <typename T>
    struct VectorField {
        Matrix<array<T, deltas.size()>, Height, Width> v;

        T &add(int x, int y, int dx, int dy, T dv) {
            return get(x, y, dx, dy) += dv;
        }

        T &get(int x, int y, int dx, int dy) {
            size_t i = ranges::find(deltas, pair(dx, dy)) - deltas.begin();
            assert(i < deltas.size());
            return v[x][y][i];
        }
    };

    struct ParticleParams {
        char type;
        PType cur_p;
        array<VelocityType, deltas.size()> v;

        void swap_with(StaticFluidSimulation &sim, int x, int y) {
            swap(sim.field[x][y], type);
            swap(sim.p[x][y], cur_p);
            swap(sim.velocity.v[x][y], v);
        }
    };

    const array<Fixed<>, rhoSize> rho;
    const Fixed<> g;

    Matrix<char, Height, Width> field;

    Matrix<PType, Height, Width> p, old_p;

    VectorField<VelocityType> velocity;
    VectorField<VelocityFlowType> velocity_flow;

    Matrix<int, Height, Width> last_use = {};
    Matrix<int, Height, Width> dirs = {};
    int UT = 0;

    mt19937 rnd = mt19937(1337);

    Fixed<> random01() {
        return Fixed<>::from_raw((rnd() & ((1LL << Fixed<>::K) - 1)));
    }

    Fixed<> move_prob(int x, int y) {
        Fixed<> sum = 0;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
                continue;
            }
            auto v = velocity.get(x, y, dx, dy);
            if (v < 0) {
                continue;
            }
            sum += v;
        }
        return sum;
    }

    tuple<Fixed<>, bool, pair<int, int>> propagate_flow(int x, int y,
                                                        Fixed<> lim) {
        last_use[x][y] = UT - 1;
        Fixed<> ret = 0;
        for (auto [dx, dy] : deltas) {
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] != '#' && last_use[nx][ny] < UT) {
                auto cap = velocity.get(x, y, dx, dy);
                auto flow = velocity_flow.get(x, y, dx, dy);
                if (flow == cap) {
                    continue;
                }
                auto vp = min(VelocityType(lim), cap - VelocityType(flow));
                if (last_use[nx][ny] == UT - 1) {
                    velocity_flow.add(x, y, dx, dy, vp);
                    last_use[x][y] = UT;
                    return {vp, 1, {nx, ny}};
                }
                auto [t, prop, end] = propagate_flow(nx, ny, vp);
                ret += t;
                if (prop) {
                    velocity_flow.add(x, y, dx, dy, t);
                    last_use[x][y] = UT;
                    return {t, prop && end != pair(x, y), end};
                }
            }
        }
        last_use[x][y] = UT;
        return {ret, 0, {0, 0}};
    }

    void propagate_stop(int x, int y, bool force = false) {
        if (!force) {
            bool stop = true;
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 &&
                    velocity.get(x, y, dx, dy) > 0) {
                    stop = false;
                    break;
                }
            }
            if (!stop) {
                return;
            }
        }
        last_use[x][y] = UT;
        for (auto [dx, dy] : deltas) {
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] == '#' || last_use[nx][ny] == UT ||
                velocity.get(x, y, dx, dy) > 0) {
                continue;
            }
            propagate_stop(nx, ny);
        }
    }

    bool propagate_move(int x, int y, bool is_first) {
        last_use[x][y] = UT - is_first;
        bool ret = false;
        int nx = -1, ny = -1;
        do {
            std::array<Fixed<>, deltas.size()> tres;
            Fixed<> sum = 0;
            for (size_t i = 0; i < deltas.size(); ++i) {
                auto [dx, dy] = deltas[i];
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
                    tres[i] = sum;
                    continue;
                }
                auto v = velocity.get(x, y, dx, dy);
                if (v < 0) {
                    tres[i] = sum;
                    continue;
                }
                sum += v;
                tres[i] = sum;
            }

            if (sum == 0) {
                break;
            }

            Fixed<> p = random01() * sum;
            size_t d = std::ranges::upper_bound(tres, p) - tres.begin();

            auto [dx, dy] = deltas[d];
            nx = x + dx;
            ny = y + dy;
            assert(velocity.get(x, y, dx, dy) > 0 && field[nx][ny] != '#' &&
                   last_use[nx][ny] < UT);

            ret = (last_use[nx][ny] == UT - 1 || propagate_move(nx, ny, false));
        } while (!ret);
        last_use[x][y] = UT;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 &&
                velocity.get(x, y, dx, dy) < 0) {
                propagate_stop(nx, ny);
            }
        }
        if (ret) {
            if (!is_first) {
                ParticleParams pp{};
                pp.swap_with(*this, x, y);
                pp.swap_with(*this, nx, ny);
                pp.swap_with(*this, x, y);
            }
        }
        return ret;
    }
};
