# simulates

C++23 simulation building blocks. Plain state, ordered systems, typed components, events and grids of any dimension. No renderer or domain model attached.

![simulates example](.github/examples.png)

```sh
cmake -S . -B build
cmake --build build
./build/simulates_examples
```

The [examples](examples) cover cellular automata, 3D motion and a small settlement economy. Link `simulates::simulates` through `add_subdirectory` or an installed CMake package.

Systems run in dependency order on one thread. Defer entity changes during iteration; flush them between systems. State and persistence belong to the simulation you build.
