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

namespace detail {
class execution_scope {
  public:
    explicit execution_scope(bool& running) : running_(running) { running_ = true; }
    ~execution_scope() { running_ = false; }
    execution_scope(const execution_scope&) = delete;
    execution_scope& operator=(const execution_scope&) = delete;

  private:
    bool& running_;
};

template <class state_type> class dependency_order {
  public:
    explicit dependency_order(const std::vector<system<state_type>>& systems)
        : systems_(systems), marks_(systems.size()) {
        order_.reserve(systems.size());
    }

    std::vector<std::size_t> compile() {
        for (std::size_t index = 0; index < systems_.size(); ++index)
            visit(index);
        return std::move(order_);
    }

  private:
    enum class mark { unseen, visiting, complete };
    const std::vector<system<state_type>>& systems_;
    std::vector<mark> marks_;
    std::vector<std::size_t> order_;

  private:
    std::size_t dependency_index(const std::string& name) const {
        const auto found = std::ranges::find(systems_, name, &system<state_type>::name);
        if (found == systems_.end())
            throw std::invalid_argument("Unknown system dependency: " + name);
        return static_cast<std::size_t>(found - systems_.begin());
    }

    void visit(std::size_t index) {
        if (marks_[index] == mark::complete)
            return;
        if (marks_[index] == mark::visiting)
            throw std::invalid_argument("System dependency cycle at " + systems_[index].name);
        marks_[index] = mark::visiting;
        for (const auto& dependency : systems_[index].after)
            visit(dependency_index(dependency));
        marks_[index] = mark::complete;
        order_.push_back(index);
    }
};
}

template <class state_type> class schedule {
  public:
    void add(system<state_type> entry) {
        if (running_)
            throw std::logic_error("Cannot change a running schedule");
        if (entry.name.empty() || !entry.run || entry.every == 0)
            throw std::invalid_argument("A system needs a name, function and positive interval");
        if (std::ranges::find(systems_, entry.name, &system<state_type>::name) != systems_.end())
            throw std::invalid_argument("Duplicate system: " + entry.name);
        systems_.push_back(std::move(entry));
        order_.clear();
    }
    void compile() {
        if (running_)
            throw std::logic_error("Cannot compile a running schedule");
        order_ = detail::dependency_order(systems_).compile();
    }
    void run(state_type& state, step time) {
        if (!std::isfinite(time.seconds) || time.seconds <= 0)
            throw std::invalid_argument("Step duration must be positive and finite");
        if (running_)
            throw std::logic_error("Cannot reenter a schedule");
        if (order_.size() != systems_.size())
            compile();
        const detail::execution_scope scope(running_);
        for (auto index : order_) {
            auto& entry = systems_[index];
            if (time.tick % entry.every == 0)
                entry.run(state, time);
        }
    }

  private:
    std::vector<system<state_type>> systems_;
    std::vector<std::size_t> order_;
    bool running_{};
};
}
