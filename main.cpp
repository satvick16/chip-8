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

void setupFont(uint8_t* ram)
{
    uint8_t fontset[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    // Store fontset in memory
    int ramIndex = START_FONT;

    for (int i = 0; i < sizeof(fontset); i++)
    {
        ram[ramIndex] = fontset[i];
        ramIndex++;
    }
}

void readRom(uint8_t* ram)
{
    // Read ROM and store in RAM
    std::string filePath = "test.ch8";
    std::ifstream romFile(filePath, std::ios::binary);

    if (romFile.is_open())
    {
        std::string data;

        while (romFile)
            data.push_back(romFile.get());

        for (int i = 0; i < data.size(); i++)
            ram[START_ROM + i] = data[i];
    }
}

void instruction8XYN(uint8_t* vars, uint8_t X, uint8_t Y, uint8_t N)
{
    // 8XY_
    switch (N)
    {
        case 0: // set
            vars[X] = vars[Y];
            break;
        case 1: // or
            vars[X] |= vars[Y];
            break;
        case 2: // and
            vars[X] &= vars[Y];
            break;
        case 3: // xor
            vars[X] ^= vars[Y];
            break;
        case 4: // add
            {
                uint8_t temp = vars[X] + vars[Y];
                vars[NUM_VARS - 1] = (temp > 255) ? 1 : 0;
                vars[X] += vars[Y];
                break;
            }
        case 5: // subtract
            vars[NUM_VARS - 1] = (vars[X] > vars[Y]) ? 1 : 0;
            vars[X] -= vars[Y];
            break;
        case 7: // subtract
            vars[NUM_VARS - 1] = (vars[Y] > vars[X]) ? 1 : 0;
            vars[X] = vars[Y] - vars[X];
            break;
        case 6: // shift
            {
                vars[X] = vars[Y];
                bool outBit = vars[X] & 1;
                vars[X] >>= 1;
                vars[NUM_VARS - 1] = (outBit) ? 1 : 0;
                break;
            }
        case 14: // shift
            {
                vars[X] = vars[Y];
                bool outBit = (vars[X] & (1 << 7)) >> 7;
                vars[X] <<= 1;
                vars[NUM_VARS - 1] = (outBit) ? 1 : 0;
                break;
            }
        default:
            break;
    }
}

void instructionDXYN(SDL_Window* window, SDL_Renderer* renderer, uint8_t* vars, uint8_t* I, uint8_t X, uint8_t Y, uint8_t N)
{
    uint8_t xIni = vars[X] & (GRID_WIDTH - 1);
    uint8_t xCoord = xIni;
    uint8_t yCoord = vars[Y] & (GRID_HEIGHT - 1);

    vars[NUM_VARS - 1] = 0;

    for (int i = 0; i < N; i++) {
        xCoord = xIni;
        uint8_t spriteRow = *(I + i);

        for (int j = 0; j < 8; j++) {
            // If you reach right edge of screen, stop drawing row
            if (xCoord >= GRID_WIDTH)
                break;

            bool spriteBit = (spriteRow & (1 << (7 - j))) != 0;

            // Read the pixel color at (xCoord, yCoord)
            Uint32 pixel;
            SDL_Rect rect = { xCoord, yCoord, 1, 1 };
            SDL_RenderReadPixels(renderer, &rect, SDL_PIXELFORMAT_ARGB8888, &pixel, sizeof(pixel));

            // Extract color components
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixel, SDL_GetWindowSurface(window)->format, &r, &g, &b, &a);

            // Check bit
            bool windowBit = (r == 255 && g == 255 && b == 255);

            // Update display accordingly
            if (spriteBit && windowBit)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                SDL_Rect pixelRect = {xCoord * PIXEL_SIZE, yCoord * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
                SDL_RenderFillRect(renderer, &pixelRect);

                vars[NUM_VARS - 1] = 1;
            }
            else if (spriteBit && !windowBit)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                SDL_Rect pixelRect = {xCoord * PIXEL_SIZE, yCoord * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
                SDL_RenderFillRect(renderer, &pixelRect);
            }

            xCoord++;
        }

        // Stop if you reach bottom edge of screen
        if (yCoord >= GRID_HEIGHT)
            break;

        yCoord++;
    }
}

void instructionCXNN(uint8_t* vars, uint8_t X, uint8_t NN)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint16_t> dist(0, std::numeric_limits<uint8_t>::max());
    
    uint8_t randomValue = static_cast<uint8_t>(dist(gen));

    vars[X] = randomValue & NN;
}

void instructionFX(
    uint8_t* vars, uint8_t right, uint8_t X, 
    uint8_t soundTimer, uint8_t delayTimer, 
    uint8_t* I, bool keyDown, uint8_t* pc, uint8_t keyHex)
{
    switch (right)
    {
        case 0x07:
            vars[X] = delayTimer;
            break;
        case 0x15:
            delayTimer = vars[X];
            break;
        case 0x18:
            soundTimer = vars[X];
            break;
        case 0x1E:
            (*I) += vars[X];
            break;
        case 0x0A:
            pc -= (!keyDown) ? 2 : 0;
            vars[X] = (keyDown) ? keyHex : vars[X];
            break;
        case 0x29:
            break;
        case 0x33:
            break;
        default:
            break;
    }
}

int main()
{
    uint8_t ram[MEMORY_SIZE] = {0}; // memory
    std::stack<uint8_t*> stack;

    uint8_t delayTimer = 0x00;
    uint8_t soundTimer = 0x00;

    uint8_t* pc = &ram[START_ROM]; // program counter: current instruction
    uint8_t* I; // index register: memory locations

    uint8_t vars[NUM_VARS]; // variable registers (V0 - VF)

    setupFont(ram);

    readRom(ram);

    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Create window and renderer
    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        GRID_WIDTH * PIXEL_SIZE,
        GRID_HEIGHT * PIXEL_SIZE,
        0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    // Main loop flag
    bool running = true;

    // Event handler
    SDL_Event event;

    // Keyboard status
    SDL_Scancode scancode;
    bool keyDown = false;
    uint8_t keyHex;

    // Keyboard mapping
    std::map<SDL_Scancode, uint8_t> keyMap = {
        {SDL_SCANCODE_1, 1}, {SDL_SCANCODE_2, 2}, {SDL_SCANCODE_3, 3}, {SDL_SCANCODE_4, 12},
        {SDL_SCANCODE_Q, 4}, {SDL_SCANCODE_W, 5}, {SDL_SCANCODE_E, 6}, {SDL_SCANCODE_R, 13},
        {SDL_SCANCODE_A, 7}, {SDL_SCANCODE_S, 8}, {SDL_SCANCODE_D, 9}, {SDL_SCANCODE_F, 14},
        {SDL_SCANCODE_Z, 10}, {SDL_SCANCODE_X, 0}, {SDL_SCANCODE_C, 11}, {SDL_SCANCODE_V, 15}
    };

    // Main loop
    while (running) {
        // Fetch instruction using 2 adjacent bytes
        uint8_t left = *pc;
        uint8_t right = *(pc + 1);

        uint16_t full = (left << 8) + right;

        // std::cout << "0x" << std::setfill('0') << std::setw(4) << std::hex << full << std::endl;

        uint8_t instructionType = (left & 0xF0) >> 4;

        uint8_t X = left & 0x0F;
        uint8_t Y = (right & 0xF0) >> 4;
        uint8_t N = right & 0x0F;

        uint8_t NN = right;
        uint16_t NNN = (X << 8) + right;

        pc += 2;

        // Poll for events
        while (SDL_PollEvent(&event)) {
            // Check for quit event
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                if (event.type == SDL_KEYDOWN)
                {
                    keyDown = true;
                    scancode = event.key.keysym.scancode;
                    keyHex = keyMap[scancode];

                    // std::cout << scancode << std::endl;
                }
                else
                {
                    keyDown = false;
                }
            }
        }

        switch (instructionType)
        {
            case 0:
                if (right == 0xE0)
                {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                    SDL_RenderClear(renderer);
                }
                else if (right == 0xEE)
                {
                    pc = stack.top();
                    stack.pop();
                }
                break;
            case 1:
                // 1NNN
                pc = &ram[NNN];
                break;
            case 2:
                // 2NNN
                stack.push(pc);
                pc = &ram[NNN];
                break;
            case 3:
                // 3XNN
                pc += (vars[X] == NN) ? 2 : 0;
                break;
            case 4:
                // 4XNN
                pc += (vars[X] != NN) ? 2 : 0;
                break;
            case 5:
                // 5XY0
                pc += (vars[X] == vars[Y]) ? 2 : 0;
                break;
            case 6:
                // 6XNN
                vars[X] = NN;
                break;
            case 7:
                // 7XNN
                vars[X] += NN;
                break;
            case 8:
                instruction8XYN(vars, X, Y, N);
                break;
            case 9:
                // 9XY0
                pc += (vars[X] == vars[Y]) ? 2 : 0;
                break;
            case 10: // A
                // ANNN
                I = &ram[NNN];
                break;
            case 11: // B
                // BNNN, BXNN
                pc = (oldBCommand) ? &ram[NNN + vars[0]] : pc = &ram[NNN + vars[X]];
                break;
            case 12: // C
                instructionCXNN(vars, X, NN);
                break;
            case 13: // D
                instructionDXYN(window, renderer, vars, I, X, Y, N);
                break;
            case 14: // E
                if (right == 0x9E)
                {
                    pc += (keyHex == vars[X]) ? 2 : 0;
                }
                else if (right == 0xA1)
                {
                    pc += (keyHex != vars[X]) ? 2 : 0;
                }
                break;
            case 15: // F
                instructionFX(vars, right, X, soundTimer, delayTimer, I, keyDown, pc, keyHex);
                break;
            default:
                break;
        }

        // Update screen
        SDL_RenderPresent(renderer);

        // Delay to cap frame rate
        SDL_Delay(FRAME_DELAY);

        if (delayTimer > 0) { delayTimer--; }
        if (soundTimer > 0) { soundTimer--; }

        if (soundTimer > 0)
        {
            // Beep
        }
    }

    // Cleanup and exit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
