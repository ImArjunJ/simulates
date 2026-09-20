#include <iostream>
int life();
double particles();
int civilisation();
int main() {
    std::cout << "Life: " << life() << " living cells\n"
              << "Free fall: " << particles() << " m\n"
              << "Civilisation: " << civilisation() << " people\n";
}
