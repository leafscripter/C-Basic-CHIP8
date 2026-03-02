#include <stdint.h>
#include <stdbool.h>

#ifndef CHIP8_H
#define CHIP8_H

#define START_ADDR 0x200
#define MEM_BOUND 4096
#define FONT_COUNT 80

typedef struct {
  uint8_t mem[MEM_BOUND]; 
  uint8_t screen[64 * 32];
  uint16_t pc;
  uint16_t I;
  uint16_t stack[16];
  uint8_t sp;
  uint8_t d_timer;
  uint8_t s_timer;
  uint8_t v_reg[16];
  uint8_t keys[16];
} CPU;

void emu_init(CPU *c);
bool emu_load_rom(CPU *c; const char *path);
void emu_cycle(CPU *c);
void emu_update_timers(CPU *c);


