#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "display.h"
#include "input.h"

#define DEBUG_OPCODES 0

#if DEBUG_OPCODES
#define OPCODE_LOG(...) printf(__VA_ARGS__)
#else
#define OPCODE_LOG(...)
#endif

Chip8* startup(void) {
    srand(time(NULL));

    printf("allocating memory for chip8...\n");
    Chip8* chip = (Chip8*)calloc(1, sizeof(Chip8));

    if (chip == NULL)
        fprintf(stderr, "Error: while the startup process of Chip8.\n");

    chip->pc = 0x200;

    return chip;
}

void opcode_print(uint16_t opcode) {
    printf("0x%04X\t", opcode);
}

uint16_t opcode_fetch(Chip8* chip, bool increase_pc) {
    uint8_t* ram = chip->ram;
    uint16_t* pc = &chip->pc;
    uint16_t opcode = ((uint16_t)ram[*pc] << 8) | ram[*pc + 1];
    if (increase_pc) *pc += 2;
    return opcode;
}

uint16_t opcode_decode_execute(Chip8* chip, uint16_t opcode) {
    // we are doing it here to avoid a more difficult bug tracking
    uint8_t I = MASK_I(opcode);
    uint8_t X = MASK_X(opcode);
    uint8_t Y = MASK_Y(opcode);
    uint8_t N = MASK_N(opcode);
    uint8_t NN = MASK_NN(opcode);
    uint16_t NNN = MASK_NNN(opcode);

    OPCODE_LOG("\nOpcode 0x%04X at PC 0x%04X\n", opcode, chip->pc - 2);

    switch (I) {
        case 0x0:
            OPCODE_LOG("0NNN: system/display instruction\n");
            if (NNN == 0x0E0) {
                OPCODE_LOG("  00E0: clear screen\n");
                display_clear();
                chip->draw_flag = true;
            } else if (NNN == 0x0EE) {
                OPCODE_LOG("  00EE: return from subroutine\n");
                chip->pc = stack_pop(chip);
            } else {
                OPCODE_LOG("  unsupported 0NNN instruction\n");
            }
            break;
        case 0x1:
            OPCODE_LOG("1NNN: PC <- 0x%03X\n", NNN);
            chip->pc = NNN;
            break;
        case 0x2: {
            uint16_t ret;
            OPCODE_LOG("2NNN: call 0x%03X, return address 0x%04X\n", NNN, chip->pc);
            ret = stack_push(chip, chip->pc);
            if (ret == 0) {
                OPCODE_LOG("  return address pushed\n");
                chip->pc = NNN;
            } else {
                OPCODE_LOG("  stack push failed\n");
            }
            break;
        }
        case 0x3:
            OPCODE_LOG("3XNN: skip if V%X (0x%02X) == 0x%02X\n", X, chip->V[X], NN);
            if (chip->V[X] == NN) {
                OPCODE_LOG("  true: skip next instruction\n");
                chip->pc += 2;
            } else {
                OPCODE_LOG("  false: continue\n");
            }
            break;
        case 0x4:
            OPCODE_LOG("4XNN: skip if V%X (0x%02X) != 0x%02X\n", X, chip->V[X], NN);
            if (chip->V[X] != NN) {
                OPCODE_LOG("  true: skip next instruction\n");
                chip->pc += 2;
            } else {
                OPCODE_LOG("  false: continue\n");
            }
            break;
        case 0x5:
            OPCODE_LOG("5XY0: skip if V%X (0x%02X) == V%X (0x%02X)\n", X, chip->V[X], Y, chip->V[Y]);
            if (chip->V[X] == chip->V[Y]) {
                OPCODE_LOG("  true: skip next instruction\n");
                chip->pc += 2;
            } else {
                OPCODE_LOG("  false: continue\n");
            }
            break;
        case 0x6:
            OPCODE_LOG("6XNN: V%X <- 0x%02X\n", X, NN);
            chip->V[X] = NN;
            break;
        case 0x7:
            OPCODE_LOG("7XNN: V%X (0x%02X) += 0x%02X\n", X, chip->V[X], NN);
            chip->V[X] += NN;
            break;
        case 0x8:
            OPCODE_LOG("8XYN: arithmetic/logical operation N=0x%X\n", N);
            switch (N) {
                case 0x0:
                    OPCODE_LOG("  8XY0: V%X <- V%X\n", X, Y);
                    chip->V[X] = chip->V[Y];
                    break;
                case 0x1:
                    OPCODE_LOG("  8XY1: V%X <- V%X | V%X\n", X, X, Y);
                    chip->V[X] |= chip->V[Y];
                    break;
                case 0x2:
                    OPCODE_LOG("  8XY2: V%X <- V%X & V%X\n", X, X, Y);
                    chip->V[X] &= chip->V[Y];
                    break;
                case 0x3:
                    OPCODE_LOG("  8XY3: V%X <- V%X ^ V%X\n", X, X, Y);
                    chip->V[X] ^= chip->V[Y];
                    break;
                case 0x4:
                    OPCODE_LOG("  8XY4: V%X <- V%X + V%X\n", X, X, Y);
                    uint8_t carry = ((uint16_t)(chip->V[X] + chip->V[Y]) > 0xFF) ? 1 : 0;
                    chip->V[X] += chip->V[Y];
                    chip->V[CARRY_REG] = carry;
                    break;
                case 0x5:
                    OPCODE_LOG("  8XY5: V%X <- V%X - V%X\n", X, X, Y);
                    uint8_t carry_sub = (chip->V[X] >= chip->V[Y]) ? 1 : 0;
                    chip->V[X] -= chip->V[Y];
                    chip->V[CARRY_REG] = carry_sub;
                    break;
                case 0x6:
                    OPCODE_LOG("  8XY6: V%X <- V%X >> 1\n", X, Y);
                    uint8_t carry_shr = chip->V[X] & 1;  // Salva il bit meno significativo
                    chip->V[X] = chip->V[Y] >> 1;
                    chip->V[CARRY_REG] = carry_shr;
                    break;
                case 0x7:
                    OPCODE_LOG("  8XY7: V%X <- V%X - V%X\n", X, Y, X);
                    uint8_t carry_subn = (chip->V[Y] >= chip->V[X]) ? 1 : 0;
                    chip->V[X] = chip->V[Y] - chip->V[X];
                    chip->V[CARRY_REG] = carry_subn;
                    break;
                case 0xE:
                    OPCODE_LOG("  8XYE: V%X <- V%X << 1\n", X, Y);
                    uint8_t carry_shl = (chip->V[X] >> 7) & 1;  // Salva il bit più significativo
                    chip->V[X] = chip->V[Y] << 1;
                    chip->V[CARRY_REG] = carry_shl;
                    break;
                default:
                    OPCODE_LOG("  unsupported 8XYN instruction\n");
                    break;
            }
            break;
        case 0x9:
            OPCODE_LOG("9XY0: skip if V%X (0x%02X) != V%X (0x%02X)\n", X, chip->V[X], Y, chip->V[Y]);
            if (chip->V[X] != chip->V[Y]) {
                OPCODE_LOG("  true: skip next instruction\n");
                chip->pc += 2;
            } else {
                OPCODE_LOG("  false: continue\n");
            }
            break;
        case 0xA:
            OPCODE_LOG("ANNN: I <- 0x%03X\n", NNN);
            chip->I_reg = NNN;
            break;
        case 0xB:
            OPCODE_LOG("BNNN: PC <- 0x%03X + V0 (0x%02X)\n", NNN, chip->V[0]);
            chip->pc = NNN + chip->V[0];
            break;
        case 0xC: {
            uint8_t random_byte = rand() & 0xFF;
            OPCODE_LOG("CXNN: V%X <- random 0x%02X & 0x%02X\n", X, random_byte, NN);
            chip->V[X] = random_byte & NN;
            break;
        }
        case 0xD: {
            uint8_t start_x = chip->V[X] % 64;
            uint8_t start_y = chip->V[Y] % 32;

            OPCODE_LOG("DXYN: draw %u-byte sprite at V%X=%u, V%X=%u\n", N, X, start_x, Y, start_y);
            chip->V[CARRY_REG] = 0;

            for (uint8_t row = 0; row < N; row++) {
                uint8_t sprite_byte = chip->ram[chip->I_reg + row];
                uint8_t y = (start_y + row) % 32;

                for (uint8_t column = 0; column < 8; column++) {
                    uint8_t sprite_bit = (sprite_byte >> (7 - column)) & 1;
                    uint8_t x = (start_x + column) % 64;

                    if (sprite_bit == 0) continue;

                    if (display->pixels[x][y] == 1) {
                        OPCODE_LOG("  pixel collision at (%u, %u): VF <- 1\n", x, y);
                        chip->V[CARRY_REG] = 1;
                    } else {
                        OPCODE_LOG("  draw pixel at (%u, %u)\n", x, y);
                    }

                    display->pixels[x][y] ^= 1;
                }
            }
            chip->draw_flag = true;
            break;
        }
        case 0xE:
            OPCODE_LOG("EXNN: keyboard instruction\n");
            if (NN == 0x9E) {
                OPCODE_LOG("  EX9E: skip if key 0x%X is pressed\n", chip->V[X]);
                if (*key_pressed == chip->V[X]) {
                    OPCODE_LOG("    true: skip next instruction\n");
                    chip->pc += 2;
                } else {
                    OPCODE_LOG("    false: continue\n");
                }
            } else if (NN == 0xA1) {
                OPCODE_LOG("  EXA1: skip if key 0x%X is not pressed\n", chip->V[X]);
                if (*key_pressed != chip->V[X]) {
                    OPCODE_LOG("    true: skip next instruction\n");
                    chip->pc += 2;
                } else {
                    OPCODE_LOG("    false: continue\n");
                }
            } else {
                OPCODE_LOG("  unsupported EXNN instruction\n");
            }
            break;
        case 0xF:
            OPCODE_LOG("FXNN: timer/memory instruction\n");
            if (NN == 0x07) {
                OPCODE_LOG("  FX07: V%X <- delay timer (%u)\n", X, chip->delay_timer);
                chip->V[X] = chip->delay_timer;
            } else if (NN == 0x15) {
                OPCODE_LOG("  FX15: delay timer <- V%X (%u)\n", X, chip->V[X]);
                chip->delay_timer = chip->V[X];
            } else if (NN == 0x18) {
                OPCODE_LOG("  FX18: sound timer <- V%X (%u)\n", X, chip->V[X]);
                chip->sound_timer = chip->V[X];
            } else if (NN == 0x1E) {
                OPCODE_LOG("  FX1E: I (0x%04X) += V%X (0x%02X)\n", chip->I_reg, X, chip->V[X]);
                chip->I_reg += chip->V[X];
            } else if (NN == 0x0A) {
                OPCODE_LOG("  FX0A: wait for a key\n");
                if (*key_pressed > 0xF) {
                    OPCODE_LOG("    no key: repeat this instruction\n");
                    chip->pc -= 2;
                    break;
                }
                OPCODE_LOG("    key 0x%X received: V%X <- key\n", *key_pressed, X);
                chip->V[X] = *key_pressed;
            } else if (NN == 0x29) {
                OPCODE_LOG("  FX29: I <- font sprite for char in V%X\n", X);
                chip->I_reg = chip->V[X] * 5;
            } else if (NN == 0x33) {
                uint8_t digit1 = 0, digit2 = 0, digit3 = 0;

                OPCODE_LOG("  FX33: store BCD of V%X (0x%02X) at I=0x%04X\n", X, chip->V[X], chip->I_reg);
                digit3 = chip->V[X] % 10;
                digit2 = ((chip->V[X] - digit3) / 10) % 10;
                digit1 = (((chip->V[X] - digit3) / 10) - digit2) / 10;

                chip->ram[chip->I_reg] = digit1;
                chip->ram[chip->I_reg + 1] = digit2;
                chip->ram[chip->I_reg + 2] = digit3;
            } else if (NN == 0x55) {
                OPCODE_LOG("  FX55: store V0 through V%X at I=0x%04X\n", X, chip->I_reg);
                for (uint8_t i = 0; i <= X; i++) {
                    chip->ram[chip->I_reg + i] = chip->V[i];
                }
            } else if (NN == 0x65) {
                OPCODE_LOG("  FX65: load V0 through V%X from I=0x%04X\n", X, chip->I_reg);
                for (uint8_t i = 0; i <= X; i++) {
                    chip->V[i] = chip->ram[chip->I_reg + i];
                }
            } else {
                OPCODE_LOG("  unsupported FXNN instruction\n");
            }
            break;
        default:
            OPCODE_LOG("unsupported opcode group 0x%X\n", I);
            break;
    }

    return 0;
}

void ram_print(Chip8* chip) {
    if (chip == NULL) {
        fprintf(stderr, "can't output ram content because Chip8 data structure pointer is null\n");
        return;
    }

    for (uint16_t i = 0; i < RAM_BYTES; i++) {
        if (i % 16 == 0) printf("0x%04X: ", i);
        printf("0x%02X%s", chip->ram[i], i % 16 == 15 ? "\n" : " ");
    }
}

void chip_dump(Chip8* chip, bool ram) {
    if (chip == NULL) {
        fprintf(stderr, "can't dump Chip8 state because the pointer is null\n");
        return;
    }

    printf("\n========== CHIP-8 STATE ==========\n");
    printf("PC:          0x%04X\n", chip->pc);
    printf("I:           0x%04X\n", chip->I_reg);
    printf("SP:          %u\n", chip->sp);
    printf("Delay timer: %u\n", chip->delay_timer);
    printf("Sound timer: %u\n", chip->sound_timer);

    printf("\nOPCODE: ");
    opcode_print(opcode_fetch(chip, false));
    printf("\n");

    printf("\nRegisters:\n");
    for (uint8_t i = 0; i < 16; i++) {
        printf("V%X: 0x%02X%s", i, chip->V[i], i % 4 == 3 ? "\n" : "\t");
    }

    printf("\nStack:\n");
    for (uint8_t i = 0; i < STACK_SIZE; i++) {
        printf("[%u]: 0x%04X%s", i, chip->stack[i], i % 4 == 3 ? "\n" : "\t");
    }

    if (ram) {
        printf("\nRAM:\n");
        for (uint16_t i = 0; i < RAM_BYTES; i++) {
            if (i % 16 == 0) printf("0x%04X: ", i);
            printf("%02X ", chip->ram[i]);
            if (i % 16 == 15) printf("\n");
        }
    }

    printf("===================================\n");
}
