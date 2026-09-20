#pragma once
#include <cmath>
#include <random>
#include <stdexcept>

namespace simulates {
using random_engine = std::mt19937;

inline double unit(random_engine& engine) {
    return static_cast<double>(engine() >> 8) / 16777216.0;
}

inline double uniform(random_engine& engine, double low, double high) {
    if (!std::isfinite(low) || !std::isfinite(high) || low > high || !std::isfinite(high - low))
        throw std::invalid_argument("Invalid random interval");
    return low + (high - low) * unit(engine);
}
}
