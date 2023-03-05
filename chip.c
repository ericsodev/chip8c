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
    chip->updateCounter = 0;

    chip->sp = 0;
    chip->i = 0;
    chip->pc = 0x200;

    Display *display = malloc(sizeof(Display));
    display->pixels = calloc(DISPLAY_WIDTH * DISPLAY_HEIGHT, sizeof(char));
    chip->display = display;

    Keypad *keypad = malloc(sizeof(Keypad));
    keypad->pad = malloc(16 * sizeof(char));
    keypad->keyPress = 0;
    chip->keypad = keypad;

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
        // Skip if VX == NN
        if (c->v[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
        {
            c->pc += 2;
        }
        break;
    case 0x4000:
        // Skip if VX != NN
        if (c->v[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
        {
            c->pc += 2;
        }
        break;
    case 0x5000:
        // Skip if VX == VY
        if (c->v[(opcode & 0x0F00) >> 8] == c->v[(opcode & 0x00F0) >> 4])
        {
            c->pc += 2;
        }
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
        x = (opcode & 0x0F00) >> 8;
        y = (opcode & 0x00F0) >> 4;
        switch (opcode & 0x000F)
        {
        case 0:
            // VX = VY
            c->v[x] = c->v[y];
            break;
        case 1:
            // VX = VX OR VY
            c->v[x] = c->v[x] | c->v[y];
            break;
        case 2:
            // VX = VX AND VY
            c->v[x] = c->v[x] & c->v[y];
            break;
        case 3:
            // VX = VX XOR VY
            c->v[x] = c->v[x] ^ c->v[y];
            break;
        case 4:
            // VX = VX + VY, VF = carry
            c->v[0xF] = (c->v[x] + c->v[y]) >> 8;
            c->v[x] = (c->v[x] + c->v[y]) & 0xFF;
            break;
        case 5:
            // VX = VX - VY, VF = carry
            c->v[0xF] = 0;
            if (c->v[x] >= c->v[y])
            {

                c->v[0xF] = 1;
            }
            c->v[x] = c->v[x] - c->v[y];
            break;
        case 6:
            // Shift VX right, VF is shifted bit
            c->v[0xF] = c->v[x] & 1;
            c->v[x] >>= 1;
            break;
        case 7:
            // VX = VY - VX, VF = carry
            c->v[0xF] = 0;
            if (c->v[y] >= c->v[x])
            {

                c->v[0xF] = 1;
            }
            c->v[x] = c->v[y] - c->v[x];
            break;
        case 0xE:
            // Shift VX left, VF is shifted bit
            c->v[0xF] = c->v[x] & 128;
            c->v[x] <<= 1;
            break;
        }
        break;
    case 0x9000:
        // Skip if VX != VY
        if (c->v[(opcode & 0x0F00) >> 8] != c->v[(opcode & 0x00F0) >> 4])
        {
            c->pc += 2;
        }
        break;
    case 0xA000:
        // set I to NNN
        c->i = opcode & 0x0FFF;
        break;
    case 0xB000:
        // Jump to NNN
        c->pc = opcode & 0x0FFF;
        break;
    case 0xC000:
        // Set VX to random number AND NN
        c->v[(opcode & 0x0F00) >> 8] = (rand() % 256) & (0x00FF & opcode);
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
        switch (opcode & 0xFF)
        {
        case 0x9E:
            if (c->keypad->pad[c->v[(opcode & 0x0F00) >> 8]] == 1)
            {
                c->pc += 2;
            }
            break;
        case 0xA1:
            if (c->keypad->pad[c->v[(opcode & 0x0F00) >> 8]] == 0)
            {
                c->pc += 2;
            }
            break;
        }
        break;
    case 0xF000:
        x = (opcode & 0x0F00) >> 8;
        switch (opcode & 0x00FF)
        {
        case 0x07:
            c->v[x] = c->delayTimer;
            break;
        case 0x15:
            c->delayTimer = c->v[x];
            break;
        case 0x18:
            c->soundTimer = c->v[x];
            break;
        case 0x1E:
            c->v[0xF] = (c->v[x] + c->i) / 4096;
            c->i = (c->i + c->v[x]) % 4096;
            break;
        case 0x0A:
            // find which key was pressed

            for (int i = 0; i < 16; i++)
            {
                if (c->keypad->pad[i] == 1)
                {

                    c->v[(opcode & 0x0F00) >> 8] = i;
                    c->pc += 2;
                    break;
                }
            }
            c->pc -= 2;
            break;
        case 0x29:
            c->i = (c->v[x] & 0xF) * 5 + 0x50;
            break;
        case 0x55:
            // load V0-VX (inclusive) to memory at i..i+x
            for (int i = 0; i < x + 1; i++)
            {
                c->mem[c->i + i] = c->v[i];
            }
            break;
        case 0x33:
            // decimal conversion
            c->mem[c->i] = c->v[x] / 100;
            c->mem[c->i + 1] = (c->v[x] / 10) % 10;
            c->mem[c->i + 2] = c->v[x] % 10;
            break;
        case 0x65:
            // load V0-VX (inclusive) from memory at i..i+x
            for (int i = 0; i < x + 1; i++)
            {
                c->v[i] = c->mem[c->i + i];
            }
            break;
        }
        break;
    }
}