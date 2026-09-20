#include <array>
#include <simulates/registry.hpp>
#include <simulates/simulation.hpp>

struct particle {
    std::array<double, 3> position{}, velocity{};
};
using particle_world = simulates::registry<particle>;

double particles() {
    particle_world world;
    const auto falling = simulates::create(world);
    simulates::emplace<particle>(world, falling, particle{{0, 10, 0}, {1, 0, 2}});
    simulates::simulation<particle_world> model{.time = simulates::make_clock(.001)};
    simulates::add(model.systems, {.name = "gravity", .run = [](particle_world& world, simulates::step time) {
                                       simulates::each<particle>(world, [&](auto, particle& body) {
                                           body.velocity[1] -= 9.81 * time.seconds;
                                       });
                                   }});
    simulates::add(model.systems, {.name = "integrate",
                                   .run =
                                       [](particle_world& world, simulates::step time) {
                                           simulates::each<particle>(world, [&](auto, particle& body) {
                                               for (std::size_t axis = 0; axis < 3; ++axis)
                                                   body.position[axis] += body.velocity[axis] * time.seconds;
                                           });
                                       },
                                   .after = {"gravity"}});
    simulates::advance(model, world, 1000);
    return simulates::get<particle>(world, falling)->position[1];
}
