#include <map>
#include <stack>
#include <random>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <iostream>

#include <unistd.h>
#include <SDL2/SDL.h>

#include "constants.hpp"

class CHIP8
{
public:
    uint8_t ram[MEMORY_SIZE]; // memory
    std::stack<uint8_t*> stack;

    uint8_t delayTimer;
    uint8_t soundTimer;

    uint8_t* pc; // program counter: current instruction
    uint8_t* I; // index register: memory locations

    uint8_t vars[NUM_VARS]; // variable registers (V0 - VF)

    // Create window and renderer
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Main loop flag
    bool running;

    // Event handler
    SDL_Event event;

    // Keyboard status
    SDL_Scancode scancode;
    bool keyDown;
    uint8_t keyHex;

    // Keyboard mapping
    std::map<SDL_Scancode, uint8_t> keyMap;

    CHIP8();

    void setupFont();
    void readRom();

    void setupGUI();

    void cycle();
    void handleEvents();
    
    void instruction8XYN(uint8_t X, uint8_t Y, uint8_t N);
    void instructionDXYN(uint8_t X, uint8_t Y, uint8_t N);
    void instructionCXNN(uint8_t X, uint8_t NN);
    void instructionFX(uint8_t right, uint8_t X);
    
    void frameCleanup();
    void fullCleanup();

    bool isRunning();
};
