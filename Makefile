CC = gcc
CFLAGS = -Wall -Wextra $(shell sdl2-config --cflags)
LIBS = $(shell sdl2-config --libs)

SRC = main.c cpu.c
TARGET = chip8

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
