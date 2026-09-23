#include <functional>
#include <simulates/events.hpp>
#include <simulates/registry.hpp>
#include <simulates/simulation.hpp>

namespace {
struct settlement {
    int people, grain;
};
struct harvest {
    simulates::entity destination;
    int amount;
};
struct civilisation_state {
    simulates::registry<settlement> settlements;
    simulates::events<harvest> harvests;
};

void harvest_settlement(simulates::events<harvest>& deliveries, simulates::entity id,
                        const settlement& village) {
    deliveries.push_back({id, village.people * 3});
}
void feed_settlement(simulates::entity, settlement& village) {
    village.grain -= village.people;
}
void harvest_fields(civilisation_state& state, simulates::step) {
    state.settlements.each<settlement>(std::bind_front(harvest_settlement, std::ref(state.harvests)));
}
void store_grain(civilisation_state& state, simulates::step) {
    for (const auto& delivery : simulates::take(state.harvests))
        if (auto* village = state.settlements.get<settlement>(delivery.destination))
            village->grain += delivery.amount;
}
void consume_grain(civilisation_state& state, simulates::step) {
    state.settlements.each<settlement>(feed_settlement);
}
}

int civilisation() {
    civilisation_state state;
    const auto village = state.settlements.create();
    state.settlements.emplace<settlement>(village, 12, 0);
    simulates::simulation<civilisation_state> model(1);
    model.add({.name = "harvest", .run = harvest_fields, .every = 3});
    model.add({.name = "store", .run = store_grain, .after = {"harvest"}});
    model.add({.name = "consume", .run = consume_grain, .after = {"store"}});
    model.advance(state, 12);
    return state.settlements.get<settlement>(village)->people;
}
