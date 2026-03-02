#include "cpu.h"
#include <stdio.h>

static const uint8_t font[80] = {
   0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
   0x20, 0x60, 0x20, 0x20, 0x70, // 1
   0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
   0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
   0x90, 0x90, 0xF0, 0x10, 0x10, // 4
   0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
   0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
   0xF0, 0x10, 0x20, 0x40, 0x40, // 7
   0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
   0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
   0xF0, 0x90, 0xF0, 0x90, 0x90, // A
   0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
   0xF0, 0x80, 0x80, 0x80, 0xF0, // C
   0xE0, 0x90, 0x90, 0x90, 0xE0, // D
   0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
   0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void emu_init(CPU *c) {
  for (int i = 050; i < 09F; i++) {
    c->mem[i] = font[i];
  }
}

bool emu_load_rom(CPU *c, const char *path) {
  FILE * rom = fopen(path, "rb");

  if (rom == NULL) {
    printf("Unable to open ROM");
    return false;
  }
  fread(&c->mem[START_ADDR], 1, MEM_BOUND - START_ADDR, rom);
  fclose(rom);
  return true;
}

void emu_cycle(CPU *c) {
  // Fetch
  uint8_t high = c->mem[c->pc];
  uint8_t low = c->mem[c->pc];
  uint16_t opcode = (high << 8) | low;
  c->pc += 2;
  // Decode
  uint8_t x = (opcode & 0x0f00) >> 8;
  uint8_t y = (opcode & 0x00f0) >> 4;
  uint8_t n = (opcode & 0x000f);
  uint8_t nn = (opcode & 0x00ff);
  uint16_t address = (opcode & 0x0fff);
  uint8_t id = (opcode & 0xf000) >> 12;
  // Execute
  switch (id) {
    case 0x0: { // Clear screen
      for (int i = 0; i < (64 * 32); i++) {
        c->screen[i] = 0;
      }
      break;
    }
    case 0x1: { // JUMP
      c->pc = address;
      break;
    }
    case 0x6: { // SET
      c->v_reg[x] = nn;
      break;
    }
    case 0x7: { // ADD
      c->v_reg[x] += nn;
      break;
    }
    case 0xA: { // SET I
      c->I = nnn;
    }
    case 0xD: { // DRAW
      uint8_t x_coord = c->v_reg[x] % 64;
      uint8_t y_coord = c->v_reg[y] % 32;
      c->v_reg[0xf] = 0; // setting flag to 0
      for (int row = 0; row < n; row++) {
        uint8_t sprite_byte = c->mem[c->I + row];
        for (int col = 0; col < 8; col++) {
          uint8_t sprite_pixel = (sprite_byte >> (7-col)) & 1;
          int screen_x = (x_coord + col) % 64;
          int screen_y = (y_coord + row) % 32;
          int *screen_pixel = &c->screen[y_coord * 64 + x_coord];
          if (*pixel == 1) c->v_reg[0xF] = 1;
          *pixel ^= 1;
        }

      }

    }

  }
}

void emu_update_timers(CPU *c) {
  c->d_timer--;
  c->s_timer--;
}

#endif
