#include "chip8.hpp"

int main()
{
    CHIP8 chip8;

    chip8.setupGUI();

    while (chip8.isRunning()) {
        chip8.handleEvents();
        chip8.cycle();
        chip8.frameCleanup();
    }

    chip8.fullCleanup();

    return 0;
}
