#include <stdio.h>
#include "cpu.h"
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdint.h>

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

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) running = 0;
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = 0;
    }
      // Emulator loop here
      emu_cycle(&cpu);
      emu_update_timers(&cpu);

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
