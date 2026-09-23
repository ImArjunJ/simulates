#include <array>
#include <functional>
#include <simulates/registry.hpp>
#include <simulates/simulation.hpp>

namespace {
struct particle {
    std::array<double, 3> position{}, velocity{};
};
using particle_world = simulates::registry<particle>;

void accelerate_particle(double seconds, simulates::entity, particle& body) {
    body.velocity[1] -= 9.81 * seconds;
}
void integrate_particle(double seconds, simulates::entity, particle& body) {
    for (std::size_t axis = 0; axis < 3; ++axis)
        body.position[axis] += body.velocity[axis] * seconds;
}
void apply_gravity(particle_world& world, simulates::step time) {
    world.each<particle>(std::bind_front(accelerate_particle, time.seconds));
}
void integrate(particle_world& world, simulates::step time) {
    world.each<particle>(std::bind_front(integrate_particle, time.seconds));
}
}

double particles() {
    particle_world world;
    const auto falling = world.create();
    world.emplace<particle>(falling, particle{{0, 10, 0}, {1, 0, 2}});
    simulates::simulation<particle_world> model(.001);
    model.add({.name = "gravity", .run = apply_gravity});
    model.add({.name = "integrate", .run = integrate, .after = {"gravity"}});
    model.advance(world, 1000);
    return world.get<particle>(falling)->position[1];
}
