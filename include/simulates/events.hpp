#pragma once
#include <utility>
#include <vector>

namespace simulates {
template <class event> using events = std::vector<event>;

template <class event> events<event> take(events<event>& pending) {
    return std::exchange(pending, {});
}
}
