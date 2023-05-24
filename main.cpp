#include <stack>
#include <random>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "constants.hpp"

void ramSnapshot(uint8_t ram[])
{
    for (int i = 0; i < 4096 / 2; i++)
    {
        // std::cout << std::hex << static_cast<int>(ram[i]) << " ";
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ram[i]) << static_cast<int>(ram[i + 1]) << " ";
    }

    std::cout << std::endl;

    return;
}

void printUint8(uint8_t x)
{
    for (int i = 0; i < 8; i++)
    {
        std::cout << ((x & (1 << (7 - i))) ? 1 : 0);
    }

    std::cout << std::endl;
}

int main()
{
    uint8_t ram[MEMORY_SIZE] = {0}; // memory
    std::stack<short int> stack;

    uint8_t delayTimer = 0x00;
    uint8_t soundTimer = 0x00;

    uint8_t* pc = &ram[START_ROM]; // program counter: current instruction
    uint8_t* I; // index register: memory locations

    uint8_t vars[NUM_VARS]; // variable registers (V0 - VF)

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

    // ramSnapshot(ram);

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

    int ctr = 0;

    // Main loop
    while (running) {
        ctr++;

        // Fetch instruction using 2 adjacent bytes
        uint8_t left = *pc;
        uint8_t right = *(pc + 1);

        uint16_t full = (left << 8) + right;

        std::cout << "0x" << std::setfill('0') << std::setw(4) << std::hex << full << std::endl;

        uint8_t instructionType = (left & 0xF0) >> 4;

        uint8_t X = left & 0x0F;
        uint8_t Y = (right & 0xF0) >> 4;
        uint8_t N = right & 0x0F;

        uint8_t NN = right;
        uint16_t NNN = (X << 8) + right;

        pc += 2;

        // Use keyboard scancodes

        // Poll for events
        while (SDL_PollEvent(&event)) {
            // Check for quit event
            if (event.type == SDL_QUIT)
                running = false;
        }

        // Draw stuff here

        switch (instructionType)
        {
            case 0:
                // 00E0
                if (left == 0x00 && right == 0xE0)
                {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                    SDL_RenderClear(renderer);
                }
                break;
            case 1:
                // 1NNN
                pc = &ram[NNN];
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
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
                        vars[X] += vars[Y]; // TODO: check for overflow
                        break;
                    case 5: // subtract
                        vars[X] -= vars[Y]; // TODO: carry flag
                        break;
                    case 7: // subtract
                        vars[X] = vars[Y] - vars[X]; // TODO: carry flag
                        break;
                    default:
                        break;
                }
                break;
            case 9:
                break;
            case 10: // A
                // ANNN
                I = &ram[NNN];
                break;
            case 11: // B
                break;
            case 12: // C
                // CXNN
                {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<uint16_t> dist(0, std::numeric_limits<uint8_t>::max());
                    
                    uint8_t randomValue = static_cast<uint8_t>(dist(gen));

                    vars[X] = randomValue & NN;
                    break;
                }
            case 13: // D
                // DXYN
                {
                    uint8_t xIni = vars[X] & (GRID_WIDTH - 1);
                    uint8_t yIni = vars[Y] & (GRID_HEIGHT - 1);

                    uint8_t xCoord = xIni;
                    uint8_t yCoord = yIni;

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
                            } else if (spriteBit && !windowBit)
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
                    break;
                }
            case 14: // E
                break;
            case 15: // F
                break;
            default:
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                SDL_RenderClear(renderer);
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

        // if (ctr == 5)
        // {
        //     sleep(10);
        //     break;
        // }
    }

    // Cleanup and exit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
