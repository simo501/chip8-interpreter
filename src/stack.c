#include "chip8.h"
#include <stdio.h>    

uint16_t stack_push(Chip8* chip, uint16_t e) {
    if (chip->sp == STACK_SIZE) {
        fprintf(stderr, "Error: stack push failed, stack is full!\n");
        return 1;
    }
    chip->stack[chip->sp] = e;
    chip->sp++;
    return 0;
}

uint16_t stack_pop(Chip8* chip) {
    if (chip->sp == 0) return chip->stack[chip->sp];
    chip->sp--;
    return chip->stack[chip->sp];
}
