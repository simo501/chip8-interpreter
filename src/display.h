#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

typedef struct {
    uint8_t pixels[64][32];
}Display;

extern Display* display;

uint8_t display_init(void);
void display_clear(void);
void display_dump(void);

#endif
