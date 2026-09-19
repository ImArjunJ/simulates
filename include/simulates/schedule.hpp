#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace simulates {
struct step {
    std::uint64_t tick;
    double seconds;
};

template <class state_type> struct system {
    std::string name;
    std::move_only_function<void(state_type&, step)> run;
    std::vector<std::string> after;
    std::uint64_t every{1};
};

template <class state_type> struct schedule {
    std::vector<system<state_type>> systems;
    std::vector<std::size_t> order;
    bool running{};
};

template <class state_type> void add(schedule<state_type>& plan, system<state_type> entry) {
    if (plan.running)
        throw std::logic_error("Cannot change a running schedule");
    if (entry.name.empty() || !entry.run || entry.every == 0)
        throw std::invalid_argument("A system needs a name, function and positive interval");
    if (std::ranges::any_of(plan.systems, [&](const auto& existing) { return existing.name == entry.name; }))
        throw std::invalid_argument("Duplicate system: " + entry.name);
    plan.systems.push_back(std::move(entry));
    plan.order.clear();
}

template <class state_type> void compile(schedule<state_type>& plan) {
    if (plan.running)
        throw std::logic_error("Cannot compile a running schedule");
    std::vector<std::size_t> order;
    std::vector<unsigned char> marks(plan.systems.size());
    auto visit = [&](auto&& self, std::size_t index) -> void {
        if (marks[index] == 2)
            return;
        if (marks[index] == 1)
            throw std::invalid_argument("System dependency cycle at " + plan.systems[index].name);
        marks[index] = 1;
        for (const auto& dependency : plan.systems[index].after) {
            auto found = std::ranges::find(plan.systems, dependency, &system<state_type>::name);
            if (found == plan.systems.end())
                throw std::invalid_argument("Unknown system dependency: " + dependency);
            self(self, static_cast<std::size_t>(found - plan.systems.begin()));
        }
        marks[index] = 2;
        order.push_back(index);
    };
    for (std::size_t index = 0; index < plan.systems.size(); ++index)
        visit(visit, index);
    plan.order = std::move(order);
}

template <class state_type> void run(schedule<state_type>& plan, state_type& state, step time) {
    if (!std::isfinite(time.seconds) || time.seconds <= 0)
        throw std::invalid_argument("Step duration must be positive and finite");
    if (plan.running)
        throw std::logic_error("Cannot reenter a schedule");
    if (plan.order.size() != plan.systems.size())
        compile(plan);
    struct execution_scope {
        bool& running;
        ~execution_scope() { running = false; }
    } scope{plan.running};
    plan.running = true;
    for (auto index : plan.order) {
        auto& entry = plan.systems[index];
        if (time.tick % entry.every == 0)
            entry.run(state, time);
    }
}
}
