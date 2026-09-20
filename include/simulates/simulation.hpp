#pragma once
#include "clock.hpp"
#include "schedule.hpp"
#include <limits>

namespace simulates {
template <class state_type> struct simulation {
    simulates::clock time;
    schedule<state_type> systems;
    std::uint64_t ticks{};
};

template <class state_type>
void advance(simulation<state_type>& model, state_type& state, std::uint64_t count) {
    if (count > std::numeric_limits<std::uint64_t>::max() - model.ticks)
        throw std::overflow_error("Simulation tick overflow");
    for (std::uint64_t index = 0; index < count; ++index) {
        run(model.systems, state, {model.ticks, model.time.step});
        ++model.ticks;
    }
}

template <class state_type>
int update(simulation<state_type>& model, state_type& state, double seconds, double speed = 1,
           int budget = 512) {
    const int count = accrue(model.time, seconds, speed, budget);
    advance(model, state, count);
    return count;
}

template <class state_type> void reset(simulation<state_type>& model, std::uint64_t tick = 0) {
    model.ticks = tick;
    model.time.pending = 0;
}
}
