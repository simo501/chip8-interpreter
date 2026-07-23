#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Display* display = NULL;

uint8_t display_init(void) {
    display = (Display*)calloc(1, sizeof(Display));

    if (display == NULL) {
        fprintf(stderr, "Error: failed to init display!\n");
        return 1;
    }

    return 0;
}

void display_clear(void) {
    memset(display->pixels, 0, sizeof(display->pixels));
}

void display_dump(void) {
    if (display == NULL) {
        fprintf(stderr, "can't dump display because it is not initialized\n");
        return;
    }

    uint8_t has_active_pixel = 0;
    for (uint8_t y = 0; y < 32; y++) {
        for (uint8_t x = 0; x < 64; x++) {
            if (display->pixels[x][y] != 0) {
                has_active_pixel = 1;
                break;
            }
        }
        if (has_active_pixel) break;
    }

    if (!has_active_pixel) return;

    printf("\n   ");
    for (uint8_t x = 0; x < 64; x++) {
        printf("%X", x % 16);
    }
    printf("\n");

    for (uint8_t y = 0; y < 32; y++) {
        printf("%02u ", y);
        for (uint8_t x = 0; x < 64; x++) {
            printf("%u", display->pixels[x][y]);
        }
        printf("\n");
    }
}
