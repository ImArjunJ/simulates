#pragma once
#include <array>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

namespace simulates {
template <std::size_t dimensions>
std::size_t offset(const std::array<std::size_t, dimensions>& shape,
                   const std::array<std::size_t, dimensions>& index) {
    std::size_t result = 0;
    for (std::size_t axis = 0; axis < dimensions; ++axis) {
        if (index[axis] >= shape[axis])
            throw std::out_of_range("Grid coordinate outside domain");
        result = result * shape[axis] + index[axis];
    }
    return result;
}

template <class value, std::size_t dimensions> class grid {
    static_assert(dimensions > 0);

  public:
    explicit grid(std::array<std::size_t, dimensions> shape, const value& initial = {})
        : shape_(shape), cells_(volume(shape), initial) {}
    const std::array<std::size_t, dimensions>& shape() const noexcept { return shape_; }
    decltype(auto) at(const std::array<std::size_t, dimensions>& index) {
        return cells_.at(offset(shape_, index));
    }
    decltype(auto) at(const std::array<std::size_t, dimensions>& index) const {
        return cells_.at(offset(shape_, index));
    }
    auto begin() noexcept { return cells_.begin(); }
    auto end() noexcept { return cells_.end(); }
    auto begin() const noexcept { return cells_.begin(); }
    auto end() const noexcept { return cells_.end(); }

  private:
    static std::size_t volume(const std::array<std::size_t, dimensions>& shape) {
        std::size_t size = 1;
        for (auto extent : shape) {
            if (!extent || size > std::numeric_limits<std::size_t>::max() / extent)
                throw std::invalid_argument("Invalid grid dimensions");
            size *= extent;
        }
        return size;
    }

  private:
    std::array<std::size_t, dimensions> shape_;
    std::vector<value> cells_;
};
}
