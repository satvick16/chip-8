#include "chip8.hpp"

CHIP8::CHIP8()
{
    (this->ram)[MEMORY_SIZE] = {0}; // memory
    
    this->delayTimer = 0x00;
    this->soundTimer = 0x00;

    this->pc = &ram[START_ROM]; // program counter: current instruction

    setupFont();
    readRom();
}

void CHIP8::setupFont()
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
        (this->ram)[ramIndex] = fontset[i];
        ramIndex++;
    }
}

void CHIP8::readRom()
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
            (this->ram)[START_ROM + i] = data[i];
    }
}

void CHIP8::setupGUI()
{
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Create window and renderer
    this->window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        GRID_WIDTH * PIXEL_SIZE,
        GRID_HEIGHT * PIXEL_SIZE,
        0);
    this->renderer = SDL_CreateRenderer(window, -1, 0);

    // Main loop flag
    this->running = true;

    // Event handler
    this->event;

    // Keyboard status
    this->keyDown = false;
    
    // Keyboard mapping
    this->keyMap = {
        {SDL_SCANCODE_1, 1}, {SDL_SCANCODE_2, 2}, {SDL_SCANCODE_3, 3}, {SDL_SCANCODE_4, 12},
        {SDL_SCANCODE_Q, 4}, {SDL_SCANCODE_W, 5}, {SDL_SCANCODE_E, 6}, {SDL_SCANCODE_R, 13},
        {SDL_SCANCODE_A, 7}, {SDL_SCANCODE_S, 8}, {SDL_SCANCODE_D, 9}, {SDL_SCANCODE_F, 14},
        {SDL_SCANCODE_Z, 10}, {SDL_SCANCODE_X, 0}, {SDL_SCANCODE_C, 11}, {SDL_SCANCODE_V, 15}
    };
}

void CHIP8::cycle()
{
    // Fetch instruction using 2 adjacent bytes
    uint8_t left = *(this->pc);
    uint8_t right = *((this->pc) + 1);

    uint16_t full = (left << 8) + right;

    std::cout << "0x" << std::setfill('0') << std::setw(4) << std::hex << full << std::endl;

    uint8_t instructionType = (left & 0xF0) >> 4;

    uint8_t X = left & 0x0F;
    uint8_t Y = (right & 0xF0) >> 4;
    uint8_t N = right & 0x0F;

    uint8_t NN = right;
    uint16_t NNN = (X << 8) + right;

    pc += 2;

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
        case 1: // 1NNN
            pc = &ram[NNN];
            break;
        case 2: // 2NNN
            stack.push(pc);
            pc = &ram[NNN];
            break;
        case 3: // 3XNN
            pc += ((this->vars)[X] == NN) ? 2 : 0;
            break;
        case 4: // 4XNN
            pc += ((this->vars)[X] != NN) ? 2 : 0;
            break;
        case 5: // 5XY0
            pc += ((this->vars)[X] == (this->vars)[Y]) ? 2 : 0;
            break;
        case 6: // 6XNN
            (this->vars)[X] = NN;
            break;
        case 7: // 7XNN
            (this->vars)[X] += NN;
            break;
        case 8: // 8XYN
            instruction8XYN(X, Y, N);
            break;
        case 9: // 9XY0
            pc += ((this->vars)[X] == (this->vars)[Y]) ? 2 : 0;
            break;
        case 10: // ANNN
            I = &ram[NNN];
            break;
        case 11: // BNNN, BXNN
            pc = (oldBCommand) ? &ram[NNN + (this->vars)[0]] : pc = &ram[NNN + (this->vars)[X]];
            break;
        case 12: // CXNN
            instructionCXNN(X, NN);
            break;
        case 13: // DXYN
            instructionDXYN(X, Y, N);
            break;
        case 14: // EX__
            if (right == 0x9E)
            {
                pc += (keyHex == (this->vars)[X]) ? 2 : 0;
            }
            else if (right == 0xA1)
            {
                pc += (keyHex != (this->vars)[X]) ? 2 : 0;
            }
            break;
        case 15: // F
            instructionFX(right, X);
            break;
        default:
            break;
    }
}

void CHIP8::handleEvents()
{
    // Poll for events
    while (SDL_PollEvent(&(this->event))) {
        // Check for quit event
        if ((this->event).type == SDL_QUIT)
        {
            this->running = false;
        }
        else if ((this->event).type == SDL_KEYDOWN || (this->event).type == SDL_KEYUP)
        {
            if ((this->event).type == SDL_KEYDOWN)
            {
                this->keyDown = true;
                this->scancode = (this->event).key.keysym.scancode;
                this->keyHex = this->keyMap[this->scancode];

                // std::cout << this->scancode << std::endl;
            }
            else
            {
                this->keyDown = false;
            }
        }
    }
}

void CHIP8::instruction8XYN(uint8_t X, uint8_t Y, uint8_t N)
{
    // 8XY_
    switch (N)
    {
        case 0: // set
            (this->vars)[X] = (this->vars)[Y];
            break;
        case 1: // or
            (this->vars)[X] |= (this->vars)[Y];
            break;
        case 2: // and
            (this->vars)[X] &= (this->vars)[Y];
            break;
        case 3: // xor
            (this->vars)[X] ^= (this->vars)[Y];
            break;
        case 4: // add
            {
                uint8_t temp = (this->vars)[X] + (this->vars)[Y];
                (this->vars)[NUM_VARS - 1] = (temp > 255) ? 1 : 0;
                (this->vars)[X] += (this->vars)[Y];
                break;
            }
        case 5: // subtract
            (this->vars)[NUM_VARS - 1] = ((this->vars)[X] > (this->vars)[Y]) ? 1 : 0;
            (this->vars)[X] -= (this->vars)[Y];
            break;
        case 7: // subtract
            (this->vars)[NUM_VARS - 1] = ((this->vars)[Y] > (this->vars)[X]) ? 1 : 0;
            (this->vars)[X] = (this->vars)[Y] - (this->vars)[X];
            break;
        case 6: // shift
            {
                (this->vars)[X] = (this->vars)[Y];
                bool outBit = (this->vars)[X] & 1;
                (this->vars)[X] >>= 1;
                (this->vars)[NUM_VARS - 1] = (outBit) ? 1 : 0;
                break;
            }
        case 14: // shift
            {
                (this->vars)[X] = (this->vars)[Y];
                bool outBit = ((this->vars)[X] & (1 << 7)) >> 7;
                (this->vars)[X] <<= 1;
                (this->vars)[NUM_VARS - 1] = (outBit) ? 1 : 0;
                break;
            }
        default:
            break;
    }
}

void CHIP8::instructionDXYN(uint8_t X, uint8_t Y, uint8_t N)
{
    uint8_t xIni = (this->vars)[X] & (GRID_WIDTH - 1);
    uint8_t xCoord = xIni;
    uint8_t yCoord = (this->vars)[Y] & (GRID_HEIGHT - 1);

    (this->vars)[NUM_VARS - 1] = 0;

    for (int i = 0; i < N; i++) {
        xCoord = xIni;
        uint8_t spriteRow = *((this->I) + i);

        for (int j = 0; j < 8; j++) {
            // If you reach right edge of screen, stop drawing row
            if (xCoord >= GRID_WIDTH)
                break;

            bool spriteBit = (spriteRow & (1 << (7 - j))) != 0;

            // Read the pixel color at (xCoord, yCoord)
            Uint32 pixel;
            SDL_Rect rect = { xCoord, yCoord, 1, 1 };
            SDL_RenderReadPixels(this->renderer, &rect, SDL_PIXELFORMAT_ARGB8888, &pixel, sizeof(pixel));

            // Extract color components
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixel, SDL_GetWindowSurface(this->window)->format, &r, &g, &b, &a);

            // Check bit
            bool windowBit = (r == 255 && g == 255 && b == 255);

            // Update display accordingly
            if (spriteBit && windowBit)
            {
                SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                SDL_Rect pixelRect = {xCoord * PIXEL_SIZE, yCoord * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
                SDL_RenderFillRect(this->renderer, &pixelRect);

                (this->vars)[NUM_VARS - 1] = 1;
            }
            else if (spriteBit && !windowBit)
            {
                SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                SDL_Rect pixelRect = {xCoord * PIXEL_SIZE, yCoord * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
                SDL_RenderFillRect(this->renderer, &pixelRect);
            }

            xCoord++;
        }

        // Stop if you reach bottom edge of screen
        if (yCoord >= GRID_HEIGHT)
            break;

        yCoord++;
    }
}

void CHIP8::instructionCXNN(uint8_t X, uint8_t NN)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint16_t> dist(0, std::numeric_limits<uint8_t>::max());
    
    uint8_t randomValue = static_cast<uint8_t>(dist(gen));

    (this->vars)[X] = randomValue & NN;
}

void CHIP8::instructionFX(uint8_t right, uint8_t X)
{
    switch (right)
    {
        case 0x07:
            (this->vars)[X] = this->delayTimer;
            break;
        case 0x15:
            this->delayTimer = (this->vars)[X];
            break;
        case 0x18:
            this->soundTimer = (this->vars)[X];
            break;
        case 0x1E:
            (*(this->I)) += (this->vars)[X];
            break;
        case 0x0A:
            this->pc -= (!this->keyDown) ? 2 : 0;
            (this->vars)[X] = (this->keyDown) ? this->keyHex : (this->vars)[X];
            break;
        case 0x29:
            break;
        case 0x33:
            break;
        default:
            break;
    }
}

void CHIP8::frameCleanup()
{
    // Update screen
    SDL_RenderPresent(this->renderer);

    // Delay to cap frame rate
    SDL_Delay(FRAME_DELAY);

    if (this->delayTimer > 0) { delayTimer--; }
    if (this->soundTimer > 0) { soundTimer--; }

    if (soundTimer > 0)
    {
        // Beep
    }
}

void CHIP8::fullCleanup()
{
    // Cleanup and exit
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}

bool CHIP8::isRunning()
{
    return this->running;
}
