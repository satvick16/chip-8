#!/bin/bash

# Compile the C++ program
g++ main.cpp chip8.cpp -lSDL2 -o a.out

# Run the executable
./a.out
