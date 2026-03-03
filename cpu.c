#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

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

// Setting sane defaults before launching our emulator
void emu_init(CPU *c) {
  // Set all attributes to 0
  memset(c, 0, sizeof(CPU));

  // seeding so instruction 0xC works
  srand(time(NULL));

  // Set our starting address
  c->pc = START_ADDR;

  // Then load font into memory from 0x50 to 0x9F
  for (int i = 0; i < 80; i++) {
    c->mem[i] = font[i];
  }
}

bool emu_load_rom(CPU *c, const char *path) {
  FILE * rom = fopen(path, "rb");

  if (rom == NULL) {
    printf("Unable to open ROM\n");
    return false;
  }
  fread(&c->mem[START_ADDR], 1, MEM_BOUND - START_ADDR, rom);
  fclose(rom);
  return true;
}

void emu_cycle(CPU *c) {
  // Fetch
  uint8_t high = c->mem[c->pc];
  uint8_t low = c->mem[c->pc + 1];
  uint16_t opcode = (high << 8) | low;
  c->pc += 2;
  c->curr_op = opcode;
  // Decode
  uint8_t x = (opcode & 0x0f00) >> 8;
  uint8_t y = (opcode & 0x00f0) >> 4;
  uint8_t n = (opcode & 0x000f);
  uint8_t nn = (opcode & 0x00ff);
  uint16_t address = (opcode & 0x0fff);
  uint8_t id = (opcode & 0xf000) >> 12;
  printf("PC: %04X, opcode: %04X\n", c->pc, opcode);
  // Execute
  switch (id) {
    case 0x0: { // Clear screen
      if (opcode == 0x00e0) {
        for (int i = 0; i < (64 * 32); i++) {
          c->screen[i] = 0;
        }
      } else if (opcode == 0x00ee){ // 0x00ee
        c->sp--; // pop the last item off the stack 
        c->pc = c->stack[c->sp];
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
      c->I = address;
      break;
    }
    case 0xD: { // DRAW
      uint8_t x_coord = c->v_reg[x] % 64;
      uint8_t y_coord = c->v_reg[y] % 32;
      c->v_reg[0xf] = 0; // setting flag to 0
      for (int row = 0; row < n; row++) {
        // One byte is one sprite in CHIP8
        // Retrieve each byte from memory using the pointer I
        uint8_t byte = c->mem[c->I + row];
        for (int col = 0; col < 8; col++) {
          uint8_t bit = (byte >> (7-col)) & 1; // getting every bit (pixel)
          int screen_x = (x_coord + col) % 64;
          int screen_y = (y_coord + row) % 32;
          uint8_t *pixel = &c->screen[screen_y * 64 + screen_x]; // getting pixels from screen
          if (bit) {
            if (*pixel == 1) c->v_reg[0xF] = 1; // Set a flag if screen pixel is also toggled
            *pixel ^= 1; // toggle on and off depending on initial state
          } 
        }
      }
      break;
    }
    case 0x2: { // SUBROUTINE
      c->sp++;
      c->stack[c->sp] = c->pc;
      c->pc = address;
      break;
    }
    case 0x3: { // skip instruction if VX = nn
      if (c->v_reg[x] == nn) c->pc+=2;
      break;
    }
    case 0x4: { // skip instruction if VX != nn
      if (c->v_reg[x] != nn) c->pc+=2;
      break;
    }
    case 0x5: { // skip instruction if VX = VY
      if (c->v_reg[x] == c->v_reg[y]) c->pc+=2;
      break;
    }
    case 0x9: { // skip instruction if VX !+ VY
      if (c->v_reg[x] != c->v_reg[y]) c->pc+=2;
      break;
    }
    case 0x8: {
      switch (n) {
        case 0x0: { // SET
          c->v_reg[x] = c->v_reg[y];
          break;
        }
        case 0x1: { // BITWISE OR
          c->v_reg[x] |= c->v_reg[y];
          break;
        }
        case 0x2: { // BINARY AND
          c->v_reg[x] &= c->v_reg[y];
          break;
        }
        case 0x3: { // BINARY XOR
          c->v_reg[x] ^= c->v_reg[y];
          break;
        }
        case 0x4: { // ADD
          uint16_t result = c->v_reg[x] + c->v_reg[y];
          c->v_reg[0xF] = (result > 255) ? 1 : 0; 
          c->v_reg[x] += c->v_reg[y];
          break;
        }
        case 0x5: { // VX = VX - VY
          uint8_t flag = (c->v_reg[x] > c->v_reg[y]) ? 1 : 0; // set flag to 1 if no underflow occurs
          c->v_reg[x] -= c->v_reg[y];
          c->v_reg[0xf] = flag;
          break;
        }
        case 0x7: { // VX = VY-VX
          uint8_t flag = (c->v_reg[y] > c->v_reg[x]) ? 1 : 0;
          c->v_reg[x] = c->v_reg[y] - c->v_reg[x];
          c->v_reg[0xf] = flag;
          break;
        }
        case 0x6: {//SHIFT RIGHT
          c->v_reg[x] = c->v_reg[y];
          c->v_reg[0xf] = c->v_reg[x] & 0x1; // set flag to the least significant bit 
          c->v_reg[x] >>= 1;
          break;
        }
        case 0xe: { // SHIFT LEFT
          c->v_reg[x] = c->v_reg[y];
          c->v_reg[0xf] = (c->v_reg[x] >> 7) & 0x1; // set flag to most significant bit
          c->v_reg[x] <<=1;
          break;
        }
      }
      break;
    }
    case 0xC: {//RANDOM
      c->v_reg[x] = (rand() % 256) & nn;
      break;
    }
    case 0xe: {
      switch (nn) {
        case 0x9e: {
          if (c->keys[c->v_reg[x]]) c->pc += 2;
          break;
        }
        case 0xa1: {
          if (!(c->keys[c->v_reg[x]])) c->pc += 2;
          break;
        }
      }
      break;
    }
    case 0xF: {
      switch (nn) {
        case 0x0A: { // Suspend program until input is provided
          bool pressed = false;
          for (int k = 0; k < 16; k++) {
            if (c->keys[k]) {
              c->v_reg[x] = k; // store the pressed key's hexadecimal value in vx
              pressed = true;
              break;
            }
          }

          // delay the next instruction until a key is pressed
          if (!pressed) c->pc -= 2;
          break;
        }
        case 0x29: { // Set I to address of font
          c->I = c->v_reg[x] * 5; // every font uses 5 bytes
          break;
        }
        case 0x33: { // Separate VX into three digits and store in mem[I]
          uint8_t num = c->v_reg[x];
          uint8_t n1 = num / 100;
          num %= 100;
          uint8_t n2 = num / 10;
          uint8_t n3 = num % 10;
          c->mem[c->I] = n1;
          c->mem[c->I + 1] = n2;
          c->mem[c->I + 2] = n3;
          break;
        }
        case 0x55: {
          uint8_t lim = c->v_reg[x];
          // Loading the contents of V0-VX into memory from I to I + X
          for (int i = 0; i <= lim; i++) {
            c->mem[c->I + i] = c->v_reg[i];
          }
          break;
        }
        case 0x65: {
          uint8_t lim = c->v_reg[x];
          // Loading the contents of memory from I + X into V0-VX
          for (int i = 0; i <= lim; i++) {
            c->v_reg[i] = c->mem[c->I + i];
          }
          break;
        }
        case 0x1e: {
          c->I += c->v_reg[x];
          break;
        }
        case 0x07: {
          c->v_reg[x] = c->d_timer;
          break;
        }
        case 0x15: {
          c->d_timer = c->v_reg[x];
          break;
        }
        case 0x18: {
          c->s_timer = c->v_reg[x];
          break;
        }
      }
    }
  }
}

void emu_update_timers(CPU *c) {
  c->d_timer--;
  c->s_timer--;
}

