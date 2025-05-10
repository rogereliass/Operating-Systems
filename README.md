# Operating-Systems

This project is a C-based simulation of a basic operating system designed for educational purposes. It showcases key OS principles such as process scheduling, memory management, and mutex-based synchronization, with a real-time graphical interface built using GTK.

## Features

- **Scheduling Algorithms**
  - First-Come-First-Serve (FCFS)
  - Round Robin (user-defined quantum)
  - Multilevel Feedback Queue (MLFQ)

- **Process Management**
  - Dynamic process loading with PCB tracking
  - Priority-based scheduling and blocking

- **Memory Management**
  - Simulated 60-word fixed-size memory
  - Allocation for instructions, variables, and PCB

- **Mutual Exclusion**
  - Mutexes for file access, user input, and screen output
  - Blocking queues and priority-based unblocking

- **Graphical User Interface (GTK)**
  - Real-time display of process states, memory layout, and queues
  - Controls for selecting scheduling algorithm, adjusting quantum, and stepping through execution

## Getting Started

### Prerequisites

- GCC Compiler
- GTK 3 development libraries  
  Install on Ubuntu/Debian:
  ```bash
  sudo apt-get install libgtk-3-dev
  
### Build & Run

```bash
make
./os_sim
