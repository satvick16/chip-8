#include <stack>
#include <random>
#include <fstream>
#include <iostream>

#include <SDL2/SDL.h>

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 32

#define START_ADDRESS 512
#define MEMORY_SIZE 4096 // 4kB
#define NUM_VARS 16

#define START_FONT 80

int main()
{
    uint8_t ram[MEMORY_SIZE]; // memory
    std::stack<short int> stack;

    uint8_t delayTimer;
    uint8_t soundTimer;

    uint8_t* pc = &ram[START_ADDRESS]; // program counter: current instruction
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
    for (int i = START_FONT; i < sizeof(fontset); i++)
    {
        ram[i] = fontset[i];
    }

    // Read ROM and store in RAM
    std::string filePath = "test.ch8";
    std::ifstream romFile(filePath, std::ios::binary);

    if (romFile.is_open())
    {
        std::string data;
        
        while (romFile)
        {
            data.push_back(romFile.get());
        }

        for (int i = 0; i < data.size(); i++)
        {
            ram[START_ADDRESS + i] = data[i];
        }
    }

    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Create window and renderer
    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    // Main loop flag
    bool running = true;

    // Event handler
    SDL_Event event;

    // Main loop
    while (running) {
        // Fetch instruction using 2 adjacent bytes
        uint8_t left = *pc;
        uint8_t right = *(pc + 1);

        uint8_t instructionType = (left & 0xF0) >> 4;

        uint8_t X = left & 0x0F;
        uint8_t Y = (right & 0xF0) >> 4;
        uint8_t N = right & 0x0F;

        uint8_t NN = right;
        uint16_t NNN = (X << 12) + right;

        pc += 2;

        // Use keyboard scancodes

        // Poll for events
        while (SDL_PollEvent(&event)) {
            // Check for quit event
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Draw stuff here

        switch (instructionType)
        {
            case 0:
                // 00E0
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
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
                    uint8_t xCoord = vars[X] & 63; // modulo
                    uint8_t yCoord = vars[Y] & 31; // modulo

                    vars[NUM_VARS - 1] = 0;

                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

                    for (int i = 0; i < 5; i++) {
                        uint8_t row = (*I) + i;

                        for (int j = 0; j < 8; j++) {

                        }
                    }

                    SDL_RenderDrawPoint(renderer, xCoord, yCoord);
                    break;
                }
            case 14: // E
                break;
            case 15: // F
                break;
            default:
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                break;
        }

        // Update screen
        SDL_RenderPresent(renderer);

        // Delay to cap frame rate
        SDL_Delay(16);

        if (delayTimer > 0) { delayTimer--; }
        if (soundTimer > 0) { soundTimer--; }

        if (soundTimer > 0)
        {
            // Beep
        }

        if (delayTimer == 0 || soundTimer == 0)
        {
            running = false;
        }
    }

    // Cleanup and exit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
