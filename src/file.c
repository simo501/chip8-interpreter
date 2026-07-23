
#include "file.h"

#include <errno.h>
#include <stdio.h>

#include "chip8.h"
#include "font.h"

// if ram == NULL it returns the ROM size.
uint16_t load_rom(char* path, uint8_t* ram) {
    FILE* fp = fopen(path, "rb");

    if (fp == NULL) {
        fprintf(stderr, "Error while reading the ROM!\n");
        return 1;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("Error while looking for the ROM's last byte.\n");
        fclose(fp);
        return 1;
    }

    long rom_size = ftell(fp);

    if (rom_size > (RAM_BYTES - RAM_OFFSET)) {
        fprintf(stderr, "ROM file is too big.\n");
        fclose(fp);
        return 1;
    }

    if (ram == NULL) {
        return rom_size;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        perror("can't push back the file pointer.\n");
        fclose(fp);
        return 1;
    }

    size_t bytes_read = fread(&ram[RAM_OFFSET], 1, (size_t)rom_size, fp);

    if (bytes_read < (size_t)rom_size) {
        fprintf(stderr, "Error while reading ROM, only %zu of %zu.\n", bytes_read, (size_t)rom_size);
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}

uint8_t load_fonts(uint8_t* ram) {
    for (uint8_t i = 0; i < 80; i++) {
        ram[i] = chip8_fontset[i];
    }
    return 0;
}
