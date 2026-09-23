#include <numeric>
#include <simulates/grid.hpp>
#include <simulates/simulation.hpp>

using board = simulates::grid<unsigned char, 2>;

void generation(board& cells, simulates::step) {
    board next(cells.shape());
    for (int row = 0; row < 9; ++row) {
        for (int column = 0; column < 9; ++column) {
            int neighbors = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    if ((dx || dy) && row + dy >= 0 && row + dy < 9 && column + dx >= 0 && column + dx < 9)
                        neighbors += cells.at({std::size_t(row + dy), std::size_t(column + dx)});
            const std::array<std::size_t, 2> position{std::size_t(row), std::size_t(column)};
            next.at(position) = neighbors == 3 || (cells.at(position) && neighbors == 2);
        }
    }
    cells = std::move(next);
}

int life() {
    board cells({9, 9});
    cells.at({4, 3}) = cells.at({4, 4}) = cells.at({4, 5}) = 1;
    simulates::simulation<board> model(1);
    model.add({.name = "generation", .run = generation});
    model.advance(cells, 2);
    return std::accumulate(cells.begin(), cells.end(), 0);
}
