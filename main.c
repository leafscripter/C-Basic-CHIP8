#include <stdio.h>
#include "cpu.h"
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <string.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE 15 

int main(int argc, char * argv[]) {
  if (argc < 2) {
    printf("Usage: %s <rom>\n", argv[0]);
    return 1;
  }

  CPU cpu;
  emu_init(&cpu);
  emu_load_rom(&cpu, argv[1]);
  
  bool debug = false;

  // Check for debug flag
  cpu.state = STATE_RUNNING; // set this as the default unless the program finds a --debug flag
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--debug") == 0) {
      cpu.state = STATE_PAUSED; // pause the emulator if a debug flag is passed
      debug = true;
      break;
    }  
  }
  

  SDL_Window *window = SDL_CreateWindow(
    "Emulator",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE,
    0
  );

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // RGB24 expresses one pixel as three bytes
  uint8_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT * 3];

  SDL_Texture *texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGB24,
    SDL_TEXTUREACCESS_STREAMING,
    SCREEN_WIDTH, SCREEN_HEIGHT
  );

  int running = 1;
  SDL_Event event;

  uint8_t keymap[16] = {
    [0x0] = SDL_SCANCODE_X,
    [0x1] = SDL_SCANCODE_1,
    [0x2] = SDL_SCANCODE_2,
    [0x3] = SDL_SCANCODE_3,
    [0x4] = SDL_SCANCODE_Q,
    [0x5] = SDL_SCANCODE_W,
    [0x6] = SDL_SCANCODE_E,
    [0x7] = SDL_SCANCODE_A,
    [0x8] = SDL_SCANCODE_S,
    [0x9] = SDL_SCANCODE_D,
    [0xA] = SDL_SCANCODE_Z,
    [0xB] = SDL_SCANCODE_C,
    [0xC] = SDL_SCANCODE_4,
    [0xD] = SDL_SCANCODE_R,
    [0xE] = SDL_SCANCODE_F,
    [0xF] = SDL_SCANCODE_V,
  };

  uint8_t prev_keys[SDL_NUM_SCANCODES] = {0};

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) running = 0;
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = 0;
    }

    // Handle input
    const uint8_t *keys = SDL_GetKeyboardState(NULL);
    for (int k = 0; k < 16; k++) {
      if (keys[keymap[k]]) cpu.keys[k] = 1;
    }

    if (debug) {
      if (keys[SDL_SCANCODE_P] && !prev_keys[SDL_SCANCODE_P]) {
        cpu.state = STATE_PAUSED;
      }
      if (keys[SDL_SCANCODE_RETURN] && !prev_keys[SDL_SCANCODE_RETURN]) {
        cpu.state = STATE_STEP;
      }
    }

    memcpy(prev_keys, keys, SDL_NUM_SCANCODES);

    // Emulator loop here
    if (cpu.state == STATE_RUNNING || cpu.state == STATE_STEP) {
      emu_cycle(&cpu);
      if (cpu.state == STATE_STEP) cpu.state = STATE_PAUSED;
    }

      // Update drawing
    for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) {
      uint8_t color = cpu.screen[i] ? 0xFF : 0x00;
      framebuffer[i * 3 + 0] = color; // R
      framebuffer[i * 3 + 1] = color; // G
      framebuffer[i * 3 + 2] = color; // B
    }

    SDL_UpdateTexture(texture, NULL, framebuffer, SCREEN_WIDTH * 3);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    

      // Implement better timing later
    SDL_Delay(16);
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
