#include <stdint.h>
#include <stdio.h>
#include <chip8.h>

#define SINGLE_STEP_MODE 0

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Bad arguments\n");
    return 1;
  }

  // Initialization
  //uint32_t screenWidth = 1600;
  //uint32_t screenHeight = 800;
  //uint32_t pixelHeight = screenHeight/CHIP8_SCREEN_HEIGHT;
  //uint32_t pixelWidth = screenWidth/CHIP8_SCREEN_WIDTH;
  uint16_t keyboard = 0;
  const uint32_t target_fps = 60;
  #if !(SINGLE_STEP_MODE)
  const uint32_t cycles_per_frame = CHIP8_CLOCK_SPEED/target_fps;
  #endif


  CHIP8_initialize();
  CHIP8_load(argv[1]);

  // Main game loop
  while (1) {

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

    // sound

      if (chip8_sound > 0) {
        chip8_sound--;
      }
      if (chip8_delay > 0) {
        chip8_delay--;
      }
  }
  return 0;
}
