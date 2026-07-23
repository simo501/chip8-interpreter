#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "chip8.h"
#include "display.h"
#include "file.h"
#include "input.h"

uint8_t window_create(Chip8* chip);
uint8_t window_update(SDL_Renderer* renderer);

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Error: no rom path specified\n");
        return 1;
    }

    Chip8* chip = startup();

    if (chip == NULL) return 1;

    if (load_fonts(chip->ram) == 1) return 1;

    if (load_rom(argv[1], chip->ram) == 1) return 1;

    if (display_init() == 1) return 1;

    if (keypad_init() == 1) return 1;

    window_create(chip);
    return 0;
}

uint8_t window_update(SDL_Renderer* renderer) {
    // creiamo il renderer per disegnare
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Colore dei pixel (bianco)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (display->pixels[x][y] == 1) {
                // scaliamo i pixels
                SDL_FRect rect = {x * 10.0f, y * 10.0f, 10.0f, 10.0f};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
    SDL_RenderPresent(renderer);

    return 0;
}

uint8_t window_create(Chip8* chip) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Error: failed to init SDL.\n");
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("CHIP-8", 640, 320, 0);
    if (!window) {
        fprintf(stderr, "Error: failed to init SDL window.\n");
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        fprintf(stderr, "Error: failed to init SDL renderer.\n");
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderVSync(renderer, 1);

    bool* running = (bool*)malloc(sizeof(bool));
    *running = true;

    // 1000 millisecondi / 700 istruzioni = ~1.42 ms per istruzione
    const double ms_per_opcode = 1000.0 / 700.0;
    const uint8_t target_fps = 1000 / 60;
    double last_fps_tick = 0;
    double accumulator = 0;
    uint64_t last_time = SDL_GetTicks();

    while (*running) {
        uint64_t current_time = SDL_GetTicks();
        accumulator += (current_time - last_time);
        last_time = current_time;

        // chip_dump(chip, false);

        while (accumulator >= ms_per_opcode) {
            process_input(running);
            // keypad_print();
            opcode_decode_execute(chip, opcode_fetch(chip, true));
            accumulator -= ms_per_opcode;
        }

        // while (SDL_GetTicks() < last_fps_tick + target_fps) {
        // }
        uint64_t next_tick = last_fps_tick + target_fps;
        uint64_t now = SDL_GetTicks();
        if (now < next_tick) {
            SDL_Delay((uint32_t)(next_tick - now));
        }
        
        if (chip->delay_timer > 0) {
            chip->delay_timer--;
        }

        if (chip->sound_timer > 0) {
            // Suona un beep finché il timer è > 0 (da implementare con SDL_Audio)
            chip->sound_timer--;
        }

        if (chip->draw_flag) {
            window_update(renderer);
            chip->draw_flag = false;
        }
        last_fps_tick = SDL_GetTicks();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
