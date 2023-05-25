# CHIP-8 Interpreter

## Summary

This is an interpreter for [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) which was a programming language created in the late 1970s which uses simple 16-bit hexadecimal instructions.

Some key features of CHIP-8 include:
* 4 kB of RAM
* 64 x 32 pixel, 60 Hz display
* 16 8-bit variable registers
* built-in font sprites
* stack for calling subroutines

Like any interpreter, the system uses the "fetch-decode-execute" loop paradigm to walk through a list of program instructions stored in a ROM, line-by-line and process/render any outputs.

## Implementation

The interpreter wsas written in C++ and the graphics were created using the [SDL](https://en.wikipedia.org/wiki/Simple_DirectMedia_Layer) library.
