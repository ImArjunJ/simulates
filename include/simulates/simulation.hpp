#pragma once
#include "clock.hpp"
#include "schedule.hpp"
#include <limits>

namespace simulates {
template <class state_type> class simulation {
  public:
    explicit simulation(double step) : time_(step) {}
    void add(system<state_type> entry) { systems_.add(std::move(entry)); }
    void compile() { systems_.compile(); }
    void advance(state_type& state, std::uint64_t count) {
        if (count > std::numeric_limits<std::uint64_t>::max() - ticks_)
            throw std::overflow_error("Simulation tick overflow");
        for (std::uint64_t index = 0; index < count; ++index) {
            systems_.run(state, {ticks_, time_.step_size()});
            ++ticks_;
        }
    }
    int update(state_type& state, double seconds, double speed = 1, int budget = 512) {
        const int count = time_.accrue(seconds, speed, budget);
        advance(state, count);
        return count;
    }
    void reset(std::uint64_t tick = 0) {
        ticks_ = tick;
        time_.reset();
    }
    std::uint64_t tick() const noexcept { return ticks_; }
    double backlog() const noexcept { return time_.backlog(); }

  private:
    simulates::clock time_;
    schedule<state_type> systems_;
    std::uint64_t ticks_{};
};
}
