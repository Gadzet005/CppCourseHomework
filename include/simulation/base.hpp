#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <ranges>
#include <simulation/common.hpp>
#include <simulation/interface.hpp>
#include <type_traits>
#include <types/fixed.hpp>
#include <vector>

/// @brief Base fluid simulation.
/// If (Height, Width) == (0, 0), then use dynamic field.
/// Otherwise use static field.
template <typename PType, typename VelocityType, typename VelocityFlowType,
          size_t Height = 0, size_t Width = 0>
class BaseFluidSimulation : virtual public FluidSimulationInterface {
protected:
    static constexpr bool isDynamic = Height == 0 && Width == 0;

    template <typename T>
    using Matrix = std::conditional<isDynamic, DynamicMatrix<T>,
                                    StaticMatrix<T, Height, Width>>::type;

    template <typename T>
    using VectorMatrix =
        std::conditional<isDynamic, DynamicVectorMatrix<T>,
                         StaticVectorMatrix<T, Height, Width>>::type;

public:
    bool step() override {
        Fixed<> total_delta_p = 0;
        // Apply external forces
        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
                if (field[x][y] == '#') continue;
                if (field[x + 1][y] != '#')
                    velocity.add(x, y, 1, 0, VelocityType(g));
            }
        }

        // Apply forces from p
        old_p = p;
        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
                if (field[x][y] == '#') continue;
                for (auto [dx, dy] : deltas) {
                    int nx = x + dx, ny = y + dy;
                    if (field[nx][ny] != '#' && old_p[nx][ny] < old_p[x][y]) {
                        PType delta_p = old_p[x][y] - old_p[nx][ny];
                        PType force = delta_p;
                        VelocityType &contr = velocity.get(nx, ny, -dx, -dy);
                        if (contr * VelocityType(rho[(int)field[nx][ny]]) >=
                            force) {
                            contr -= VelocityType(
                                force / PType(rho[(int)field[nx][ny]]));
                            continue;
                        }
                        force -= PType(contr *
                                       VelocityType(rho[(int)field[nx][ny]]));
                        contr = 0;
                        velocity.add(
                            x, y, dx, dy,
                            VelocityType(force / PType(rho[(int)field[x][y]])));
                        p[x][y] -= force / dirs[x][y];
                        total_delta_p -= force / dirs[x][y];
                    }
                }
            }
        }

        // Make flow from velocities
        velocity_flow.reset();
        bool prop = false;
        do {
            UT += 2;
            prop = 0;
            for (size_t x = 0; x < height; ++x) {
                for (size_t y = 0; y < width; ++y) {
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
        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
                if (field[x][y] == '#') continue;
                for (auto [dx, dy] : deltas) {
                    VelocityType old_v = velocity.get(x, y, dx, dy);
                    VelocityType new_v =
                        VelocityType(velocity_flow.get(x, y, dx, dy));
                    if (old_v > 0) {
                        assert(new_v <= old_v);
                        velocity.get(x, y, dx, dy) = new_v;
                        PType force = (old_v - new_v) *
                                      VelocityType(rho[(int)field[x][y]]);
                        if (field[x][y] == '.') force *= 0.8;
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
        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
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

    void print_field(std::ostream &out) const override {
        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
                out << field[x][y];
            }
            out << '\n';
        }
    }

    virtual FluidSimulationState getState() const override {
        FluidSimulationState state(this->height, this->width);
        state.g = this->g;
        state.rho = this->rho;
        state.UT = this->UT;

        for (size_t x = 0; x < this->height; ++x) {
            for (size_t y = 0; y < this->width; ++y) {
                state.field[x][y] = this->field[x][y];
                state.p[x][y] = this->p[x][y];
                state.dirs[x][y] = this->dirs[x][y];
                state.last_use[x][y] = this->last_use[x][y];

                for (size_t k = 0; k < deltas.size(); k++) {
                    state.velocity[x][y][k] = this->velocity.v[x][y][k];
                }
            }
        }
        return state;
    }

protected:
    BaseFluidSimulation(size_t height, size_t width, Fixed<> g,
                        std::array<Fixed<>, rhoSize> rho)
        : height(height), width(width), g(g), rho(rho) {}

    template <typename T>
    struct VectorField {
        VectorMatrix<T> v;

        T &add(int x, int y, int dx, int dy, T dv) {
            return get(x, y, dx, dy) += dv;
        }

        T &get(int x, int y, int dx, int dy) {
            size_t i =
                std::ranges::find(deltas, std::pair(dx, dy)) - deltas.begin();
            assert(i < deltas.size());
            return v[x][y][i];
        }

        void reset() {
            for (size_t i = 0; i < v.size(); i++) {
                for (size_t j = 0; j < v[0].size(); j++) {
                    v[i][j].fill(0);
                }
            }
        }
    };

    struct ParticleParams {
        char type;
        PType cur_p;
        std::array<VelocityType, deltas.size()> v;
        BaseFluidSimulation &sim;

        ParticleParams(BaseFluidSimulation &sim) : sim(sim) {}

        void swap_with(int x, int y) {
            std::swap(sim.field[x][y], type);
            std::swap(sim.p[x][y], cur_p);
            std::swap(sim.velocity.v[x][y], v);
        }
    };

    size_t height, width;

    const Fixed<> g;
    const std::array<Fixed<>, rhoSize> rho;

    Matrix<char> field;

    Matrix<PType> p, old_p;

    VectorField<VelocityType> velocity;
    VectorField<VelocityFlowType> velocity_flow;

    Matrix<int> last_use{};
    Matrix<int> dirs{};
    int UT = 0;

    std::mt19937_64 rnd{1337};

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

    std::tuple<Fixed<>, bool, std::pair<int, int>> propagate_flow(int x, int y,
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
                VelocityFlowType vp =
                    std::min(VelocityType(lim), cap - VelocityType(flow));
                if (last_use[nx][ny] == UT - 1) {
                    velocity_flow.add(x, y, dx, dy, vp);
                    last_use[x][y] = UT;
                    return {vp, 1, {nx, ny}};
                }
                auto [t, prop, end] = propagate_flow(nx, ny, vp);
                ret += t;
                if (prop) {
                    velocity_flow.add(x, y, dx, dy, VelocityFlowType(t));
                    last_use[x][y] = UT;
                    return {t, prop && end != std::pair(x, y), end};
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
                ParticleParams pp(*this);
                pp.swap_with(x, y);
                pp.swap_with(nx, ny);
                pp.swap_with(x, y);
            }
        }
        return ret;
    }
};
