# Memory Hierarchy Simulator

This is a C++ simulation project for modeling a memory hierarchy system, developed as part of a **Computer Architecture** course assignment.

##  Features

- User-configurable:
  - Cache sizes, block sizes, access times
  - Replacement policies: FIFO, LRU, Random
  - RAM and TLB configuration
  - Disk access time and size
- Supports three memory access patterns:
  - Sequential
  - Random
  - Looping
- Hit/miss tracking and performance reporting

##  How to Build & Run

### Build

You can compile the project with any modern C++ compiler. Example using `g++`:

```bash
g++ -std=c++11 -o memory_simulator project.cpp
```

### Run

After compiling:

```bash
./memory_simulator
```

Follow the on-screen prompts to configure the memory hierarchy and run simulations.

##  Output

- Hit/miss status per memory level
- Total access time for each address
- Final performance report:
  - Hit/Miss count and rates per cache level
  - Overall access statistics

##  File Structure

```
memory-hierarchy-simulator/
├── project.cpp     # Main C++ source code
├── README.md       # Project documentation
```

##  Course Context

This project was created for a **Computer Architecture** course assignment to simulate and analyze the behavior of multi-level memory hierarchies.

##  License

You may use this code under the terms of the MIT License *(or replace with your preferred license)*.
