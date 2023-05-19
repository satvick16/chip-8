#include <iostream>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 32
#define START_ADDRESS 512

int main() {
    uint16_t ram[4096]; // memory
    std::stack<short int> stack;

    uint8_t delayTimer;
    uint8_t soundTimer;

    uint16_t* pc = &ram[START_ADDRESS]; // program counter: current instruction
    uint16_t* I; // index register: memory locations

    uint8_t vars[16]; // variable registers (V0 - VF)

    uint8_t fontset[] = {
        0x00F0, 0x0090, 0x0090, 0x0090, 0x00F0, // 0
        0x0020, 0x0060, 0x0020, 0x0020, 0x0070, // 1
        0x00F0, 0x0010, 0x00F0, 0x0080, 0x00F0, // 2
        0x00F0, 0x0010, 0x00F0, 0x0010, 0x00F0, // 3
        0x0090, 0x0090, 0x00F0, 0x0010, 0x0010, // 4
        0x00F0, 0x0080, 0x00F0, 0x0010, 0x00F0, // 5
        0x00F0, 0x0080, 0x00F0, 0x0090, 0x00F0, // 6
        0x00F0, 0x0010, 0x0020, 0x0040, 0x0040, // 7
        0x00F0, 0x0090, 0x00F0, 0x0090, 0x00F0, // 8
        0x00F0, 0x0090, 0x00F0, 0x0010, 0x00F0, // 9
        0x00F0, 0x0090, 0x00F0, 0x0090, 0x0090, // A
        0x00E0, 0x0090, 0x00E0, 0x0090, 0x00E0, // B
        0x00F0, 0x0080, 0x0080, 0x0080, 0x00F0, // C
        0x00E0, 0x0090, 0x0090, 0x0090, 0x00E0, // D
        0x00F0, 0x0080, 0x00F0, 0x0080, 0x00F0, // E
        0x00F0, 0x0080, 0x00F0, 0x0080, 0x0080  // F
    }

    // Store fontset in memory
    for (int i = 0; i < sizeof(fontset); i++)
    {
        ram[i] = fontset[i];
    }

    // Read ROM and store in RAM
    std::string filePath = "";
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
    SDL_Window* window = SDL_CreateWindow("CHIP-8",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    // Main loop flag
    bool running = true;

    // Event handler
    SDL_Event event;

    // Main loop
    while (running) {
        /*
        fetch-decode-execute loop:
        - fetch 2-byte instruction using pc and increment pc by 2
        - decode: switch statement for first half-byte which is instruction type (nested)

        X: The second nibble. Used to look up one of the 16 registers (VX) from V0 through VF.
        Y: The third nibble. Also used to look up one of the 16 registers (VY) from V0 through VF.
        N: The fourth nibble. A 4-bit number.
        NN: The second byte (third and fourth nibbles). An 8-bit immediate number.
        NNN: The second, third and fourth nibbles. A 12-bit immediate memory address.
        
        instructions:

        00E0 (clear screen)
        1NNN (jump)
        6XNN (set register VX)
        7XNN (add value to register VX)
        ANNN (set index register I)
        DXYN (display/draw) */

        uint16_t instruction = *pc;
        pc += 2;

        uint8_t instructionType = (instruction >> 12);

        switch (instructionType)
        {
            case value1:
                // code to execute if expression matches value1
                break;
            case value2:
                // code to execute if expression matches value2
                break;
            // more cases...
            default:
                // code to execute if expression doesn't match any case
                break;
        }

        // Use keyboard scancodes

        // Poll for events
        while (SDL_PollEvent(&event)) {
            // Check for quit event
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw stuff here

        // Update screen
        SDL_RenderPresent(renderer);

        // Delay to cap frame rate
        SDL_Delay(16);

        // Decrement timers
        delayTimer--;
        soundTimer--;

        if (delayTimer == 0 || soundTimer == 0)
        {
            running = false;
        }

        // Beep
    }

    // Cleanup and exit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
