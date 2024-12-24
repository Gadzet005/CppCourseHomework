#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <queue>
#include <random>
#include <ranges>
#include <simulation/common.hpp>
#include <simulation/interface.hpp>
#include <thread/thread_pool.hpp>
#include <type_traits>
#include <types/fixed.hpp>
#include <vector>

/// @brief Base fluid simulation.
/// If (Height, Width) == (0, 0), then use dynamic field.
/// Otherwise use static field.
template <typename PType, typename VelocityType, typename VelocityFlowType,
          size_t Height = 0, size_t Width = 0>
class FluidSimulation : virtual public FluidSimulationInterface {
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
    FluidSimulation(const FluidSimulationState &state, unsigned threads = 1)
        : height(state.getFieldHeight()),
          width(state.getFieldWidth()),
          g(state.g),
          rho(state.rho),
          UT(state.UT),
          tickCount(state.tickCount),
          pool(threads) {
        // Allocate memory if simulation is dynamic.
        if constexpr (isDynamic) {
            this->field.resize(this->height, std::vector<char>(this->width));
            this->p.resize(this->height, std::vector<PType>(this->width));
            this->dirs.resize(this->height, std::vector<int>(this->width, 0));
            this->lastUse.resize(this->height,
                                 std::vector<int>(this->width, 0));
            this->velocity.v.resize(
                this->height,
                std::vector<std::array<VelocityType, deltas.size()>>(
                    this->width, std::array<VelocityType, deltas.size()>()));
            this->velocityFlow.v.resize(
                this->height,
                std::vector<std::array<VelocityFlowType, deltas.size()>>(
                    this->width,
                    std::array<VelocityFlowType, deltas.size()>()));
            this->old_p = this->p;
        }

        // Copy state.
        for (size_t x = 0; x < this->height; ++x) {
            for (size_t y = 0; y < this->width; ++y) {
                this->field[x][y] = state.field[x][y];
                this->p[x][y] = PType(state.p[x][y]);
                this->dirs[x][y] = state.dirs[x][y];
                this->lastUse[x][y] = state.lastUse[x][y];

                for (size_t k = 0; k < deltas.size(); k++) {
                    this->velocity.v[x][y][k] =
                        VelocityType(state.velocity[x][y][k]);
                }
            }
        }
    }

    bool step() override {
        PType total_delta_p = 0;

        // Apply external forces
        auto computeRow = [this](size_t x) {
            for (size_t y = 0; y < this->width; ++y) {
                if (this->field[x][y] == '#') continue;
                if (this->field[x + 1][y] != '#')
                    this->velocity.add(x, y, 1, 0, VelocityType(this->g));
            }
        };
        for (size_t x = 0; x < height; x++) {
            pool.addTask(computeRow, x);
        }
        pool.waitAll();

        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
                old_p[x][y] = p[x][y];
            }
        }

        // Apply forces from p
        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
                if (field[x][y] == '#') continue;
                for (auto [dx, dy] : deltas) {
                    int nx = x + dx, ny = y + dy;
                    if (field[nx][ny] != '#' && old_p[nx][ny] < old_p[x][y]) {
                        PType force = old_p[x][y] - old_p[nx][ny];
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
        velocityFlow.reset();
        bool any_prop;
        std::vector<std::pair<size_t, size_t>> current;
        std::vector<std::pair<size_t, size_t>> next;
        current.reserve(height * width);
        next.reserve(height * width);

        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
                if (field[x][y] != '#') {
                    current.push_back({x, y});
                }
            }
        }

        do {
            UT += 2;
            any_prop = false;

            for (auto [x, y] : current) {
                if (lastUse[x][y] != UT) {
                    auto [t, local_prop, _] = propagateFlow(x, y, 1);
                    if (t > 0) {
                        next.push_back({x, y});
                        for (auto [dx, dy] : deltas) {
                            int nx = x + dx, ny = y + dy;
                            if (field[nx][ny] != '#') {
                                next.push_back({nx, ny});
                            }
                        }
                        any_prop = true;
                    }
                } else if (flowCache[x][y] > 0) {
                    next.push_back({x, y});
                }
            }

            swap(current, next);
            next.clear();
        } while (any_prop);

        // Recalculate p with kinetic energy
        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
                if (field[x][y] == '#') continue;
                for (auto [dx, dy] : deltas) {
                    VelocityType old_v = velocity.get(x, y, dx, dy);
                    VelocityType new_v =
                        VelocityType(velocityFlow.get(x, y, dx, dy));
                    if (old_v > 0) {
                        assert(new_v <= old_v);
                        velocity.get(x, y, dx, dy) = new_v;
                        PType force = (old_v - new_v) *
                                      VelocityType(rho[(int)field[x][y]]);
                        if (field[x][y] == '.') {
                            force *= 0.8;
                        }
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
        bool prop = false;
        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
                if (field[x][y] != '#' && lastUse[x][y] != UT) {
                    if (random01() < moveProb(x, y)) {
                        prop = true;
                        propagateMove(x, y, true);
                    } else {
                        propagateStop(x, y, true);
                    }
                }
            }
        }

        if (prop) {
            tickCount++;
        }

        return prop;
    }

    void printField(std::ostream &out) const override {
        for (size_t x = 0; x < height; ++x) {
            for (size_t y = 0; y < width; ++y) {
                out << field[x][y];
            }
            out << '\n';
        }
        out << std::flush;
    }

    unsigned getTickCount() const override { return tickCount; }

    virtual FluidSimulationState getState() const override {
        FluidSimulationState state(this->height, this->width);
        state.g = this->g;
        state.rho = this->rho;
        state.UT = this->UT;
        state.tickCount = this->tickCount;

        for (size_t x = 0; x < this->height; ++x) {
            for (size_t y = 0; y < this->width; ++y) {
                state.field[x][y] = this->field[x][y];
                state.p[x][y] = this->p[x][y];
                state.dirs[x][y] = this->dirs[x][y];
                state.lastUse[x][y] = this->lastUse[x][y];

                for (size_t k = 0; k < deltas.size(); k++) {
                    state.velocity[x][y][k] = this->velocity.v[x][y][k];
                }
            }
        }

        return state;
    }

protected:
    template <typename T>
    struct VectorField {
        VectorMatrix<T> v;

        T &add(int x, int y, int dx, int dy, T dv) {
            return get(x, y, dx, dy) += dv;
        }

        T &get(int x, int y, int dx, int dy) {
            return v[x][y][getDeltaIndex(dx, dy)];
        }

        void reset() {
            for (auto &row : v) {
                for (auto &elem : row) {
                    elem.fill(0);
                }
            }
        }
    };

    struct ParticleParams {
        char type;
        PType cur_p;
        std::array<VelocityType, deltas.size()> v;
        FluidSimulation &sim;

        ParticleParams(FluidSimulation &sim) : sim(sim) {}

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
    VectorField<VelocityFlowType> velocityFlow;

    Matrix<int> lastUse{};
    Matrix<int> dirs{};
    int UT = 0;

    unsigned tickCount = 0;

    ThreadPool pool;

    std::vector<std::vector<Fixed<>>> flowCache{height,
                                                std::vector<Fixed<>>(width)};

    std::mt19937_64 rnd{1337};

    Fixed<> random01() {
        return Fixed<>::fromRaw((rnd() & ((1LL << Fixed<>::K) - 1)));
    }

    Fixed<> moveProb(int x, int y) {
        Fixed<> sum = 0;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] == '#' || lastUse[nx][ny] == UT) {
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

    std::tuple<Fixed<>, bool, std::pair<int, int>> propagateFlow(int x, int y,
                                                                 Fixed<> lim) {
        lastUse[x][y] = UT - 1;
        Fixed<> ret = 0;

        for (auto [dx, dy] : deltas) {
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] == '#' || lastUse[nx][ny] >= UT) {
                continue;
            };

            auto cap = velocity.get(x, y, dx, dy);
            auto flow = velocityFlow.get(x, y, dx, dy);
            if (flow == cap) {
                continue;
            }

            VelocityFlowType vp =
                std::min(VelocityType(lim), cap - VelocityType(flow));
            if (lastUse[nx][ny] == UT - 1) {
                velocityFlow.add(x, y, dx, dy, vp);
                lastUse[x][y] = UT;
                flowCache[x][y] = vp;
                return {vp, true, {nx, ny}};
            }

            auto [t, prop, end] = propagateFlow(nx, ny, vp);

            ret += t;
            if (prop) {
                velocityFlow.add(x, y, dx, dy, VelocityFlowType(t));
                lastUse[x][y] = UT;
                flowCache[x][y] = t;
                return {t, prop && end != std::pair(x, y), end};
            }
        }

        lastUse[x][y] = UT;
        flowCache[x][y] = ret;
        return {ret, false, {0, 0}};
    }

    void propagateStop(int x, int y, bool force = false) {
        if (!force) {
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] != '#' && lastUse[nx][ny] < UT - 1 &&
                    velocity.get(x, y, dx, dy) > 0) {
                    return;
                }
            }
        }

        lastUse[x][y] = UT;
        for (auto [dx, dy] : deltas) {
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] == '#' || lastUse[nx][ny] == UT ||
                velocity.get(x, y, dx, dy) > 0) {
                continue;
            }
            propagateStop(nx, ny);
        }
    }

    bool propagateMove(int x, int y, bool is_first) {
        lastUse[x][y] = UT - is_first;
        bool ret = false;
        int nx = -1, ny = -1;

        do {
            std::array<Fixed<>, deltas.size()> tres;
            Fixed<> sum = 0;
            for (size_t i = 0; i < deltas.size(); ++i) {
                auto [dx, dy] = deltas[i];
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] == '#' || lastUse[nx][ny] == UT) {
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
                   lastUse[nx][ny] < UT);

            ret = (lastUse[nx][ny] == UT - 1 || propagateMove(nx, ny, false));
        } while (!ret);

        lastUse[x][y] = UT;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] != '#' && lastUse[nx][ny] < UT - 1 &&
                velocity.get(x, y, dx, dy) < 0) {
                propagateStop(nx, ny);
            }
        }

        if (ret && !is_first) {
            ParticleParams pp(*this);
            pp.swap_with(x, y);
            pp.swap_with(nx, ny);
            pp.swap_with(x, y);
        }
        return ret;
    }
};
