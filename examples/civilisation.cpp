#include <simulates/events.hpp>
#include <simulates/registry.hpp>
#include <simulates/simulation.hpp>

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

int civilisation() {
    civilisation_state state;
    const auto village = simulates::create(state.settlements);
    simulates::emplace<settlement>(state.settlements, village, 12, 0);
    simulates::simulation<civilisation_state> model{.time = simulates::make_clock(1)};
    simulates::add(model.systems, {.name = "harvest",
                                   .run =
                                       [](civilisation_state& state, auto) {
                                           simulates::each<settlement>(
                                               state.settlements, [&](auto id, const settlement& village) {
                                                   state.harvests.push_back({id, village.people * 3});
                                               });
                                       },
                                   .every = 3});
    simulates::add(model.systems, {.name = "store",
                                   .run =
                                       [](civilisation_state& state, auto) {
                                           for (const auto& delivery : simulates::take(state.harvests))
                                               if (auto* village = simulates::get<settlement>(
                                                       state.settlements, delivery.destination))
                                                   village->grain += delivery.amount;
                                       },
                                   .after = {"harvest"}});
    simulates::add(model.systems, {.name = "consume",
                                   .run =
                                       [](civilisation_state& state, auto) {
                                           simulates::each<settlement>(state.settlements,
                                                                       [](auto, settlement& village) {
                                                                           village.grain -= village.people;
                                                                       });
                                       },
                                   .after = {"store"}});
    simulates::advance(model, state, 12);
    return simulates::get<settlement>(state.settlements, village)->people;
}
