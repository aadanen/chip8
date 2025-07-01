#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <chip8.h>

#define SINGLE_STEP_MODE 0

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Bad arguments\n");
    return 1;
  }

  // Initialization
  uint32_t screenWidth = 1600;
  uint32_t screenHeight = 800;
  uint32_t pixelHeight = screenHeight/CHIP8_SCREEN_HEIGHT;
  uint32_t pixelWidth = screenWidth/CHIP8_SCREEN_WIDTH;

  // for the chip8
  uint16_t keyboard = 0;
  const uint32_t target_fps = 60;
  #if !(SINGLE_STEP_MODE)
  const uint32_t cycles_per_frame = CHIP8_CLOCK_SPEED/target_fps;
  #endif
  CHIP8_initialize();
  CHIP8_load(argv[1]);

  // for sdl
  SDL_SetAppMetadata("chip8", "1.0", "");
  SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  SDL_Window *window;
  window = SDL_CreateWindow(
      "chip8",
      screenWidth,
      screenHeight,
      0
      );
  if (window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", 
         SDL_GetError());
    return 1;
  }

  SDL_Renderer *renderer;
  renderer = SDL_CreateRenderer(window, NULL);
  if (renderer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n", 
         SDL_GetError());
    return 1;
  }

  SDL_FRect bkgd;
  bkgd.x = 0;
  bkgd.y = 0;
  bkgd.w = screenWidth;
  bkgd.h = screenHeight;

  SDL_FRect chip8_pixel;
  chip8_pixel.x = 0;
  chip8_pixel.y = 0;
  chip8_pixel.w = pixelWidth;
  chip8_pixel.h = pixelHeight;


  // Main game loop
  bool done = false;
  while (!done) {
    // handle system events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        done = true;
      }
    }

    #if !(SINGLE_STEP_MODE)
    for (uint32_t i = 0; i < cycles_per_frame; i++) {
      CHIP8_cycle(keyboard);
    }
    #else
    if (1) {
      CHIP8_dump();
      CHIP8_cycle(keyboard);
    }
    #endif

    // graphics
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &bkgd);


    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    for (uint8_t i = 0; i < CHIP8_SCREEN_HEIGHT; i++) {
      for (uint8_t j = 0; j < CHIP8_SCREEN_WIDTH; j++) {
        chip8_pixel.y = i*pixelHeight;
        chip8_pixel.x = j*pixelWidth;
        if (chip8_screen[i][j]) {
          SDL_RenderFillRect(renderer, &chip8_pixel);
        }
      }
    }


    SDL_RenderPresent(renderer);


    // sound
    if (chip8_sound > 0) {
      chip8_sound--;
    }
    if (chip8_delay > 0) {
      chip8_delay--;
    }
  }
  SDL_Quit();
  return 0;
}
