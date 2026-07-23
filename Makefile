CC := cc
SDL_CFLAGS := $(shell pkg-config --cflags sdl3)
SDL_LIBS := $(shell pkg-config --libs sdl3)
CFLAGS := -std=c11 -Wall -Wextra -pedantic -Isrc $(SDL_CFLAGS)

TARGET := chip8
SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(SDL_LIBS) -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) ibm.ch8

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all run clean
