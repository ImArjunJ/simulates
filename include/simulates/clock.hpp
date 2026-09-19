#pragma once
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace simulates {
struct clock {
    double step;
    double pending{};
};

inline clock make_clock(double step) {
    if (!std::isfinite(step) || step <= 0)
        throw std::invalid_argument("Step must be positive and finite");
    return {.step = step};
}

inline int accrue(clock& time, double seconds, double speed = 1, int budget = 512) {
    if (!std::isfinite(time.step) || time.step <= 0)
        throw std::invalid_argument("Step must be positive and finite");
    if (!std::isfinite(seconds) || !std::isfinite(speed) || seconds < 0 || speed <= 0 || budget <= 0)
        return 0;
    const double added = seconds * speed;
    if (!std::isfinite(added) || !std::isfinite(time.pending + added))
        throw std::overflow_error("Simulation backlog overflow");
    time.pending += added;
    const auto tolerance = std::min(1e-10, time.step * 1e-8);
    const auto count =
        static_cast<int>(std::min(double(budget), std::floor((time.pending + tolerance) / time.step)));
    time.pending = std::max(0.0, time.pending - count * time.step);
    return count;
}
}
