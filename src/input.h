#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

#define KEYPAD_SIZE 16

extern bool* keypad;
extern uint8_t *key_pressed;

uint8_t keypad_init(void);
void keypad_print(void);
void key_pressed_get(void);

void process_input(bool *running);

#endif 
