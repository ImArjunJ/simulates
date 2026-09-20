#pragma once
#include <array>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

namespace simulates {
template <class value, std::size_t dimensions> struct grid {
    static_assert(dimensions > 0);
    std::array<std::size_t, dimensions> shape;
    std::vector<value> cells;
};

template <class value, std::size_t dimensions>
grid<value, dimensions> make_grid(std::array<std::size_t, dimensions> shape, const value& initial = {}) {
    std::size_t size = 1;
    for (auto extent : shape) {
        if (!extent || size > std::numeric_limits<std::size_t>::max() / extent)
            throw std::invalid_argument("Invalid grid dimensions");
        size *= extent;
    }
    return {shape, std::vector<value>(size, initial)};
}

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

template <class value, std::size_t dimensions>
decltype(auto) at(grid<value, dimensions>& state, const std::array<std::size_t, dimensions>& index) {
    return state.cells.at(offset(state.shape, index));
}

template <class value, std::size_t dimensions>
decltype(auto) at(const grid<value, dimensions>& state, const std::array<std::size_t, dimensions>& index) {
    return state.cells.at(offset(state.shape, index));
}
}
