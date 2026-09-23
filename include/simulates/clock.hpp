#pragma once
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace simulates {
class clock {
  public:
    explicit clock(double step) : step_(step) {
        if (!std::isfinite(step) || step <= 0)
            throw std::invalid_argument("Step must be positive and finite");
    }
    double step_size() const noexcept { return step_; }
    double backlog() const noexcept { return pending_; }
    void reset() noexcept { pending_ = 0; }
    int accrue(double seconds, double speed = 1, int budget = 512) {
        if (!std::isfinite(seconds) || !std::isfinite(speed) || seconds < 0 || speed <= 0 || budget <= 0)
            return 0;
        const double added = seconds * speed;
        if (!std::isfinite(added) || !std::isfinite(pending_ + added))
            throw std::overflow_error("Simulation backlog overflow");
        pending_ += added;
        const auto tolerance = std::min(1e-10, step_ * 1e-8);
        const auto count =
            static_cast<int>(std::min(double(budget), std::floor((pending_ + tolerance) / step_)));
        pending_ = std::max(0.0, pending_ - count * step_);
        return count;
    }

  private:
    double step_, pending_{};
};
}
