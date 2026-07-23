#ifndef FILE_H
#define FILE_H

#include <stdint.h>

uint16_t load_rom(char* path, uint8_t* ram);
uint8_t load_fonts(uint8_t* ram);

#endif 
