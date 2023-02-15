#include "chip.h"
#include <stdio.h> /* printf and fprintf */

#ifdef _WIN32
#include <SDL2/SDL.h> /* Windows-specific SDL2 library */
#else
#include <SDL2/SDL.h> /* macOS- and GNU/Linux-specific */
#endif

/* Sets constants */
#define WIDTH 660
#define HEIGHT 340
#define DELAY 3000

void draw(SDL_Renderer *ren, Display *dis)
{
    SDL_RenderClear(ren);
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    for (int x = 0; x < 64; x++)
    {
        for (int y = 0; y < 32; y++)
        {
            if (dis->pixels[y * 64 + x] == 1)
            {
                SDL_Rect rect;
                rect.h = 10;
                rect.w = 10;
                rect.x = 10 * x + 10;
                rect.y = 10 * y + 10;
                SDL_RenderFillRect(ren, &rect);
            }
        }
    }
    dis->drawFlag = 0;
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderPresent(ren);
}

int main(int argc, char **argv)
{
    /* Initialises data */
    SDL_Window *window = NULL;

    /*
     * Initialises the SDL video subsystem (as well as the events subsystem).
     * Returns 0 on success or a negative error code on failure using SDL_GetError().
     */
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
        return 1;
    }

    /* Creates a SDL window */
    window = SDL_CreateWindow("SDL Example",           /* Title of the SDL window */
                              SDL_WINDOWPOS_UNDEFINED, /* Position x of the window */
                              SDL_WINDOWPOS_UNDEFINED, /* Position y of the window */
                              WIDTH,                   /* Width of the window in pixels */
                              HEIGHT,                  /* Height of the window in pixels */
                              0);                      /* Additional flag(s) */

    /* Checks if window has been created; if not, exits program */
    if (window == NULL)
    {
        fprintf(stderr, "SDL window failed to initialise: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL)
    {
        fprintf(stderr, "SDL renderer failed to initialise: %s\n", SDL_GetError());
        return 1;
    }

    // SDL_Delay(DELAY);
    int running = 1;
    SDL_Event e;
    Chip *chip = createChip();
    FILE *fpin = fopen("test_opcode.ch8", "r");
    loadRom(fpin, chip);
    while (running)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            // User requests quit
            switch (e.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN:
                break;
            }
        }
        cycle(chip);
        if (chip->display->drawFlag != 0)
        {
            draw(renderer, chip->display);
        }
        SDL_Delay(20);
    }
    /* Frees memory */
    SDL_DestroyWindow(window);
    /* Shuts down all SDL subsystems */
    SDL_Quit();

    return 0;
}