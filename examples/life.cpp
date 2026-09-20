#include <numeric>
#include <simulates/grid.hpp>
#include <simulates/simulation.hpp>

using board = simulates::grid<unsigned char, 2>;

void generation(board& cells, simulates::step) {
    auto next = simulates::make_grid<unsigned char>(cells.shape);
    for (int row = 0; row < 9; ++row) {
        for (int column = 0; column < 9; ++column) {
            int neighbors = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    if ((dx || dy) && row + dy >= 0 && row + dy < 9 && column + dx >= 0 && column + dx < 9)
                        neighbors += simulates::at(cells, {std::size_t(row + dy), std::size_t(column + dx)});
            const std::array<std::size_t, 2> position{std::size_t(row), std::size_t(column)};
            simulates::at(next, position) =
                neighbors == 3 || (simulates::at(cells, position) && neighbors == 2);
        }
    }
    cells = std::move(next);
}

int life() {
    auto cells = simulates::make_grid<unsigned char, 2>({9, 9});
    simulates::at(cells, {4, 3}) = simulates::at(cells, {4, 4}) = simulates::at(cells, {4, 5}) = 1;
    simulates::simulation<board> model{.time = simulates::make_clock(1)};
    simulates::add(model.systems, {.name = "generation", .run = generation});
    simulates::advance(model, cells, 2);
    return std::accumulate(cells.cells.begin(), cells.cells.end(), 0);
}
