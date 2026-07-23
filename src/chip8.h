#ifndef CHIP8_H
#define CHIP8_H

#define RAM_BYTES 4096
#define RAM_OFFSET 0x200

#define STACK_SIZE 16

#define CARRY_REG 0xF

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t ram[RAM_BYTES];
    uint16_t pc; // program counter
    uint8_t sp; // stack pointer
    uint8_t delay_timer; 
    uint8_t sound_timer;
    uint8_t V[16]; // general-purpose variable registers 
    uint16_t stack[STACK_SIZE];
    uint16_t I_reg; // 16-bit index register called “I” which is used to point at locations in memory
    bool draw_flag;
} Chip8;

uint16_t stack_pop(Chip8* chip);
uint16_t stack_push(Chip8* chip, uint16_t e);

// masks
#define MASK_I(opcode) (((opcode) & 0xF000) >> 12)  // (I in IXY0)) 
#define MASK_X(opcode) (((opcode) & 0x0F00) >> 8)   // (X in 8XY0)
#define MASK_Y(opcode) (((opcode) & 0x00F0) >> 4)   // (Y in 8XY0)
#define MASK_N(opcode) ((opcode) & 0x000F)          // (N in DXYN)
#define MASK_NN(opcode) ((opcode) & 0x00FF)         // (KK in 6XNN)
#define MASK_NNN(opcode) ((opcode) & 0x0FFF)        // (BNNN)

Chip8* startup(void);

// opcode 
uint16_t opcode_fetch(Chip8* chip, bool increase_pc);
uint16_t opcode_decode_execute(Chip8* chip, uint16_t opcode);
void opcode_print(uint16_t opcode);

void ram_print(Chip8* chip);
void chip_dump(Chip8* chip, bool ram);

#endif
