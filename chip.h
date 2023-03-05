#include <stdio.h>
typedef struct
{
    int updateCounter;
    unsigned char *pixels;
    char drawFlag;
} Display;

typedef struct
{
    char keyPress;
    char *pad;
} Keypad;

typedef struct
{
    unsigned char *mem;
    unsigned char *v;
    unsigned short pc;
    unsigned short i;
    unsigned short *stack; // pointer to pointer to memory
    unsigned short sp;     // stack pointer
    unsigned char delayTimer;
    unsigned char soundTimer;
    int updateCounter;
    Keypad *keypad;
    Display *display;
} Chip;

void loadRom(FILE *fpin, Chip *c);
void cycle(Chip *c);
Chip *createChip();