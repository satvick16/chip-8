#include <SDL2/SDL.h>

const int GRID_WIDTH = 64;
const int GRID_HEIGHT = 32;
const int PIXEL_SIZE = 10;  // Adjust this value to increase/decrease the pixel size

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("SDL Grid", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          GRID_WIDTH * PIXEL_SIZE, GRID_HEIGHT * PIXEL_SIZE, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Render the grid
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        for (int x = 0; x < GRID_WIDTH; ++x) {
            for (int y = 0; y < GRID_HEIGHT; ++y) {
                SDL_Rect pixelRect = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
                SDL_RenderFillRect(renderer, &pixelRect);
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
