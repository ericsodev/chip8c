#include "chip.h"
#include <stdlib.h>
#include <stdio.h>

#define MEM_SIZE 4096  // memory size in bytes
#define V_REGS_SIZE 16 // v registers
#define STACK_SIZE 16  // v registers

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

unsigned char fonts[80] = {
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

Chip *
createChip()
{
    Chip *chip = malloc(sizeof(Chip));
    chip->mem = calloc(MEM_SIZE, sizeof(char));
    chip->v = calloc(V_REGS_SIZE, sizeof(char));
    chip->stack = calloc(STACK_SIZE, sizeof(short));
    chip->delayTimer = 0;
    chip->soundTimer = 0;

    chip->sp = 0;
    chip->i = 0;
    chip->pc = 0x200;

    Display *display = malloc(sizeof(Display));
    display->pixels = calloc(DISPLAY_WIDTH * DISPLAY_HEIGHT, sizeof(char));
    chip->display = display;

    // load fonts to 0x050 to 0x0A0
    for (int i = 0; i < 80; i++)
    {
        chip->mem[0x50 + i] = fonts[i];
    }
    return chip;
}

void loadRom(FILE *fpin, Chip *c)
{
    if (fpin == NULL)
    {
        return;
    }
    // load rom to memory starting from 0x200
    char *b = malloc(sizeof(char));
    int i = 0;

    while (fread(b, 1, 1, fpin) != 0)
    {
        c->mem[0x200 + i] = *b;
        i++;
    }
    fclose(fpin);
}

void cycle(Chip *c)
{

    unsigned short opcode = c->mem[c->pc] << 8 | c->mem[c->pc + 1];

    c->pc = c->pc + 2;

    int x,
        y;
    unsigned short newAddr;
    switch (opcode & 0xF000)
    {
    case 0x0000:
        if (opcode == 0x00E0)
        {
            // clear screen
            for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
            {
                c->display->pixels[i] = 0;
            }
        }
        else if (opcode == 0x00EE)
        {
            // return from subroutine
            c->sp--;
            c->pc = c->stack[c->sp];
        }
        break;
    case 0x1000:
        // jump
        newAddr = 0x0FFF & opcode;
        c->pc = newAddr;
        break;
    case 0x2000:
        // subroutine jump

        // push current pc to stack;
        c->stack[c->sp] = c->pc;
        c->sp++;

        newAddr = 0x0FFF & opcode;
        c->pc = newAddr;
        break;
    case 0x3000:
        break;
    case 0x4000:
        break;
    case 0x5000:
        break;
    case 0x6000:
        // set VX to NN
        c->v[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        break;
    case 0x7000:
        // add NN to VX
        c->v[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        break;
    case 0x8000:
        break;
    case 0x9000:
        break;
    case 0xA000:
        // set I to NNN
        c->i = opcode & 0x0FFF;
        break;
    case 0xB000:
        break;
    case 0xC000:
        break;
    case 0xD000:
        x = c->v[(opcode & 0x0F00) >> 8] % DISPLAY_WIDTH;
        y = c->v[(opcode & 0x00F0) >> 4] % DISPLAY_HEIGHT;
        int n = opcode & 0x000F;
        char collision = 0;
        Display *d = c->display;

        // loads n bytes (as n 8-pixel rows)
        for (int i = 0; i < n; i++)
        {
            unsigned char row = c->mem[c->i + i];
            if (i + y >= DISPLAY_HEIGHT)
                break;
            // iterating y coordinates
            for (int j = 0; j < 8; j++)
            {
                if (j + x >= DISPLAY_WIDTH)
                    break;
                // iterating x coordinates
                unsigned char pixelOn = row & (0x80 >> j);
                if (pixelOn == 0)
                    continue;

                int pixelIdx = (x + j) + 64 * (y + i);
                d->drawFlag = 1;
                if (d->pixels[pixelIdx] == 1)
                {
                    collision = 1;
                    d->pixels[pixelIdx] = 0;
                }
                else
                {
                    d->pixels[pixelIdx] = 1;
                }
            }
        }
        // set VF to collision
        c->v[0xF] = collision;
        break;
    case 0xE000:
        break;
    case 0xF000:
        break;
    }
}