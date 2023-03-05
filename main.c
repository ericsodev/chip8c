#include "chip.h"
#include <stdio.h>
#include <time.h>

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
    window = SDL_CreateWindow("Chip 8 Emulator",       /* Title of the SDL window */
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
    FILE *fpin;
    if (argc == 1)
    {

        fpin = fopen("IBM Logo.ch8", "rb");
    }
    else if (argc >= 2)
    {
        // first argument is filename

        fpin = fopen(argv[1], "rb");
    }

    if (fpin == NULL)
    {

        fprintf(stderr, "Error opening ROM\n");
        return 127;
    }
    loadRom(fpin, chip);
    struct timespec ts;

    clock_t start = clock();
    while (running)
    {
        char *pad = chip->keypad->pad;
        while (SDL_PollEvent(&e) != 0)
        {
            // User requests quit
            switch (e.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            // case SDL_KEYDOWN:
            //     break;
            case SDL_KEYDOWN:
                // Only register key after release
                switch (e.key.keysym.sym)
                {
                case SDLK_1:
                    chip->keypad->keyPress = 1;
                    pad[1] = 1;
                    break;
                case SDLK_2:
                    chip->keypad->keyPress = 1;
                    pad[2] = 1;
                    break;
                case SDLK_3:
                    chip->keypad->keyPress = 1;
                    pad[3] = 1;
                    break;
                case SDLK_4:
                    chip->keypad->keyPress = 1;
                    pad[12] = 1;
                    break;
                case SDLK_q:
                    chip->keypad->keyPress = 1;
                    pad[4] = 1;
                    break;
                case SDLK_w:
                    chip->keypad->keyPress = 1;
                    pad[5] = 1;
                    break;
                case SDLK_e:
                    chip->keypad->keyPress = 1;
                    pad[6] = 1;
                    break;
                case SDLK_r:
                    chip->keypad->keyPress = 1;
                    pad[13] = 1;
                    break;
                case SDLK_a:
                    chip->keypad->keyPress = 1;
                    pad[7] = 1;
                    break;
                case SDLK_s:
                    chip->keypad->keyPress = 1;
                    pad[8] = 1;
                    break;
                case SDLK_d:
                    chip->keypad->keyPress = 1;
                    pad[9] = 1;
                    break;
                case SDLK_f:
                    chip->keypad->keyPress = 1;
                    pad[14] = 1;
                    break;
                case SDLK_z:
                    chip->keypad->keyPress = 1;
                    pad[10] = 1;
                    break;
                case SDLK_x:
                    chip->keypad->keyPress = 1;
                    pad[0] = 1;
                    break;
                case SDLK_c:
                    chip->keypad->keyPress = 1;
                    pad[11] = 1;
                    break;
                case SDLK_v:
                    chip->keypad->keyPress = 1;
                    pad[15] = 1;
                    break;
                }
                break;
            case SDL_KEYUP:
                // Only register key after release
                switch (e.key.keysym.sym)
                {
                case SDLK_1:
                    pad[1] = 0;
                    break;
                case SDLK_2:
                    pad[2] = 0;
                    break;
                case SDLK_3:
                    pad[3] = 0;
                    break;
                case SDLK_4:
                    pad[12] = 0;
                    break;
                case SDLK_q:
                    pad[4] = 0;
                    break;
                case SDLK_w:
                    pad[5] = 0;
                    break;
                case SDLK_e:
                    pad[6] = 0;
                    break;
                case SDLK_r:
                    pad[13] = 0;
                    break;
                case SDLK_a:
                    pad[7] = 0;
                    break;
                case SDLK_s:
                    pad[8] = 0;
                    break;
                case SDLK_d:
                    pad[9] = 0;
                    break;
                case SDLK_f:
                    pad[14] = 0;
                    break;
                case SDLK_z:
                    pad[10] = 0;
                    break;
                case SDLK_x:
                    pad[0] = 0;
                    break;
                case SDLK_c:
                    pad[11] = 0;
                    break;
                case SDLK_v:
                    pad[15] = 0;
                    break;
                }
                break;
            }
        }
        if ((double)(clock() - start) / CLOCKS_PER_SEC * 10 * 1000000 >= 1)
        {
            // processor at 1 MHz
            // chip->updateCounter = -1;
            cycle(chip);
            chip->delayTimer--;
            chip->soundTimer--;
        }
        if (chip->display->drawFlag != 0)
        {
            // display instantly
            draw(renderer, chip->display);
        }
        chip->display->updateCounter++;
        chip->updateCounter++;
        start = clock();
    }
    /* Frees memory */
    SDL_DestroyWindow(window);
    /* Shuts down all SDL subsystems */
    SDL_Quit();

    return 0;
}