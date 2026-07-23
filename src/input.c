#include "input.h"
#include <stdlib.h>
#include <SDL3/SDL.h>

bool* keypad = NULL;
uint8_t* key_pressed = NULL;

uint8_t keypad_init(void) {
    keypad = calloc(KEYPAD_SIZE, sizeof(bool));
    key_pressed = (uint8_t*) calloc(1, sizeof(uint8_t));

    if (keypad == NULL || key_pressed == NULL) {
        fprintf(stderr, "Error: keypad initialization failed.\n");
        return 1;
    }

    return 0;
}

void keypad_print(void) {
    for (uint8_t i = 0; i < KEYPAD_SIZE; i++) {
        if (keypad[i]) printf("keypad[%u] pressed!\n", i);
    }
}

void key_pressed_get(void) {
    *key_pressed = 0xFF;
    for (uint8_t i = 0; i < KEYPAD_SIZE; i++) {
        if (keypad[i] == true) {
            *key_pressed = i; 
            break;
        }  
    }
}

void process_input(bool *running) {
    SDL_Event event;

    // Legge tutti gli eventi in coda
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            *running = false;
        }
        // Gestisce sia la pressione che il rilascio dei tasti
        else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {

            // true se premuto, false se rilasciato
            bool is_pressed = (event.type == SDL_EVENT_KEY_DOWN);

            // Mappatura classica CHIP-8 (1234, QWER, ASDF, ZXCV)
            switch (event.key.key) {
                case SDLK_1: 
                    keypad[0x1] = is_pressed;
                    break;
                case SDLK_2: 
                    keypad[0x2] = is_pressed; 
                    break;
                case SDLK_3: 
                    keypad[0x3] = is_pressed; 
                    break;
                case SDLK_4: 
                    keypad[0xC] = is_pressed; 
                    break;
                case SDLK_Q: 
                    keypad[0x4] = is_pressed; 
                    break;
                case SDLK_W: 
                    keypad[0x5] = is_pressed; 
                    break;
                case SDLK_E: 
                    keypad[0x6] = is_pressed; 
                    break;
                case SDLK_R: 
                    keypad[0xD] = is_pressed; 
                    break;
                case SDLK_A: 
                    keypad[0x7] = is_pressed; 
                    break;
                case SDLK_S: 
                    keypad[0x8] = is_pressed; 
                    break;
                case SDLK_D: 
                    keypad[0x9] = is_pressed;    
                    break;
                case SDLK_F: 
                    keypad[0xE] = is_pressed; 
                    break;
                case SDLK_Z: 
                    keypad[0xA] = is_pressed; 
                    break;
                case SDLK_X: 
                    keypad[0x0] = is_pressed; 
                    break;
                case SDLK_C: 
                    keypad[0xB] = is_pressed; 
                    break;
                case SDLK_V: 
                    keypad[0xF] = is_pressed; 
                    break;
            } 
            key_pressed_get();
        }
    }
}
