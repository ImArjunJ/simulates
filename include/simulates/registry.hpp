#pragma once
#include <compare>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace simulates {
struct entity {
    std::uint32_t index{}, generation{};
    auto operator<=>(const entity&) const = default;
};

struct entity_slot {
    std::uint32_t generation{1};
    bool occupied{};
};

template <class component> using component_map = std::unordered_map<std::uint32_t, component>;

template <class... components> struct registry {
    std::vector<entity_slot> slots;
    std::vector<std::uint32_t> available;
    std::tuple<component_map<components>...> storage;
    std::vector<std::move_only_function<void(registry&)>> deferred;
    unsigned iteration_depth{};
};

template <class... components> bool alive(const registry<components...>& state, entity id) {
    return id.index < state.slots.size() && state.slots[id.index].occupied &&
           state.slots[id.index].generation == id.generation;
}

template <class... components> void require_mutable(const registry<components...>& state) {
    if (state.iteration_depth)
        throw std::logic_error("Defer structural changes until after iteration");
}

template <class... components> entity create(registry<components...>& state) {
    require_mutable(state);
    if (!state.available.empty()) {
        const auto index = state.available.back();
        state.available.pop_back();
        state.slots[index].occupied = true;
        return {index, state.slots[index].generation};
    }
    if (state.slots.size() >= std::numeric_limits<std::uint32_t>::max())
        throw std::overflow_error("Entity capacity exhausted");
    state.slots.push_back({1, true});
    return {static_cast<std::uint32_t>(state.slots.size() - 1), 1};
}

template <class... components> bool destroy(registry<components...>& state, entity id) {
    require_mutable(state);
    if (!alive(state, id))
        return false;
    auto& slot = state.slots[id.index];
    const auto reusable = slot.generation != std::numeric_limits<std::uint32_t>::max();
    if (reusable)
        state.available.push_back(id.index);
    std::apply([&](auto&... values) { (values.erase(id.index), ...); }, state.storage);
    slot.occupied = false;
    slot.generation += reusable;
    return true;
}

template <class component, class... components, class... arguments>
component& emplace(registry<components...>& state, entity id, arguments&&... values) {
    require_mutable(state);
    if (!alive(state, id))
        throw std::invalid_argument("Stale entity");
    auto& storage = std::get<component_map<component>>(state.storage);
    auto [entry, inserted] = storage.try_emplace(id.index, std::forward<arguments>(values)...);
    if (!inserted)
        throw std::invalid_argument("Component already exists");
    return entry->second;
}

template <class component, class... components> component* get(registry<components...>& state, entity id) {
    if (!alive(state, id))
        return nullptr;
    auto& storage = std::get<component_map<component>>(state.storage);
    auto found = storage.find(id.index);
    return found == storage.end() ? nullptr : &found->second;
}

template <class component, class... components>
const component* get(const registry<components...>& state, entity id) {
    if (!alive(state, id))
        return nullptr;
    const auto& storage = std::get<component_map<component>>(state.storage);
    auto found = storage.find(id.index);
    return found == storage.end() ? nullptr : &found->second;
}

template <class component, class... components> bool remove(registry<components...>& state, entity id) {
    require_mutable(state);
    if (!alive(state, id))
        return false;
    return std::get<component_map<component>>(state.storage).erase(id.index) != 0;
}

template <class... selected, class... components, class function>
void each(registry<components...>& state, function&& visit) {
    struct iteration_scope {
        unsigned& depth;
        ~iteration_scope() { --depth; }
    } scope{state.iteration_depth};
    ++state.iteration_depth;
    for (std::size_t index = 0; index < state.slots.size(); ++index) {
        const entity id{static_cast<std::uint32_t>(index), state.slots[index].generation};
        if (state.slots[index].occupied && (get<selected>(state, id) && ...))
            std::invoke(visit, id, *get<selected>(state, id)...);
    }
}

template <class... components, class function>
void defer(registry<components...>& state, function&& command) {
    state.deferred.emplace_back(std::forward<function>(command));
}

template <class... components> void flush(registry<components...>& state) {
    require_mutable(state);
    auto commands = std::exchange(state.deferred, {});
    for (auto& command : commands)
        command(state);
}
}
