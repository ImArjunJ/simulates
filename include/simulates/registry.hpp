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

  public:
    auto operator<=>(const entity&) const = default;
};

namespace detail {
class iteration_scope {
  public:
    explicit iteration_scope(unsigned& depth) : depth_(depth) { ++depth_; }
    ~iteration_scope() { --depth_; }
    iteration_scope(const iteration_scope&) = delete;
    iteration_scope& operator=(const iteration_scope&) = delete;

  private:
    unsigned& depth_;
};

template <class function, class... components>
void visit_components(function& visit, entity id, components*... values) {
    if ((values && ...))
        std::invoke(visit, id, *values...);
}
}

template <class... components> class registry {
  public:
    bool alive(entity id) const {
        return id.index < slots_.size() && slots_[id.index].occupied &&
               slots_[id.index].generation == id.generation;
    }
    entity create() {
        require_mutable();
        if (!available_.empty()) {
            const auto index = available_.back();
            available_.pop_back();
            slots_[index].occupied = true;
            return {index, slots_[index].generation};
        }
        if (slots_.size() >= std::numeric_limits<std::uint32_t>::max())
            throw std::overflow_error("Entity capacity exhausted");
        slots_.push_back({1, true});
        return {static_cast<std::uint32_t>(slots_.size() - 1), 1};
    }
    bool destroy(entity id) {
        require_mutable();
        if (!alive(id))
            return false;
        auto& slot = slots_[id.index];
        const bool reusable = slot.generation != std::numeric_limits<std::uint32_t>::max();
        if (reusable)
            available_.push_back(id.index);
        (std::get<component_map<components>>(storage_).erase(id.index), ...);
        slot.occupied = false;
        slot.generation += reusable;
        return true;
    }
    template <class component, class... arguments> component& emplace(entity id, arguments&&... values) {
        require_mutable();
        if (!alive(id))
            throw std::invalid_argument("Stale entity");
        auto& storage = std::get<component_map<component>>(storage_);
        auto [entry, inserted] = storage.try_emplace(id.index, std::forward<arguments>(values)...);
        if (!inserted)
            throw std::invalid_argument("Component already exists");
        return entry->second;
    }
    template <class component> component* get(entity id) {
        if (!alive(id))
            return nullptr;
        auto& storage = std::get<component_map<component>>(storage_);
        auto found = storage.find(id.index);
        return found == storage.end() ? nullptr : &found->second;
    }
    template <class component> const component* get(entity id) const {
        if (!alive(id))
            return nullptr;
        const auto& storage = std::get<component_map<component>>(storage_);
        auto found = storage.find(id.index);
        return found == storage.end() ? nullptr : &found->second;
    }
    template <class component> bool remove(entity id) {
        require_mutable();
        return alive(id) && std::get<component_map<component>>(storage_).erase(id.index) != 0;
    }
    template <class... selected, class function> void each(function&& visit) {
        const detail::iteration_scope scope(iteration_depth_);
        for (std::size_t index = 0; index < slots_.size(); ++index) {
            if (!slots_[index].occupied)
                continue;
            const entity id{static_cast<std::uint32_t>(index), slots_[index].generation};
            detail::visit_components(visit, id, get<selected>(id)...);
        }
    }
    template <class function> void defer(function&& command) {
        deferred_.emplace_back(std::forward<function>(command));
    }
    void flush() {
        require_mutable();
        auto commands = std::exchange(deferred_, {});
        for (auto& command : commands)
            command(*this);
    }

  private:
    struct entity_slot {
        std::uint32_t generation{1};
        bool occupied{};
    };
    template <class component> using component_map = std::unordered_map<std::uint32_t, component>;
    void require_mutable() const {
        if (iteration_depth_)
            throw std::logic_error("Defer structural changes until after iteration");
    }

  private:
    std::vector<entity_slot> slots_;
    std::vector<std::uint32_t> available_;
    std::tuple<component_map<components>...> storage_;
    std::vector<std::move_only_function<void(registry&)>> deferred_;
    unsigned iteration_depth_{};
};
}
