#include <stdint.h>
#include <stdio.h>
#include <raylib.h>
#include <chip8.h>

#define PIXELCOLOR RAYWHITE
#define BKGDCOLOR BLACK


uint8_t normalize_input(uint32_t keycode) {
  switch (keycode) {
    case 49:
      return 0;
    case 50:
      return 1;
    case 51:
      return 2;
    case 52:
      return 3;
    case 81:
      return 4;
    case 87:
      return 5;
    case 69:
      return 6;
    case 82:
      return 7;
    case 65:
      return 8;
    case 83:
      return 9;
    case 68:
      return 10;
    case 70:
      return 11;
    case 90:
      return 12;
    case 88:
      return 13;
    case 67:
      return 14;
    case 86:
      return 15;
  }
  return 0;
}

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
  uint32_t key = 0;
  SetTraceLogLevel(LOG_ERROR);

  CHIP8_initialize();
  CHIP8_load(argv[1]);



  InitWindow(screenWidth, screenHeight, "CHIP8");
  const uint32_t target_fps = 60;
  const uint32_t cycles_per_frame = CHIP8_CLOCK_SPEED/target_fps;

  SetTargetFPS(target_fps); 

  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose())    // Detect window close button or ESC key
  {
      // Update
      //----------------------------------------------------------------------------------
      // TODO: Update your variables here
      //----------------------------------------------------------------------------------
      key = normalize_input(GetKeyPressed());

      // 1 frame will have roughly 11 clock cycles
      // maybe later i can run the two processes seperately
      // like one thread is the screen and another thread is the emulator
      for (uint32_t i = 0; i < cycles_per_frame; i++) {
        CHIP8_cycle();
      }

      // Draw
      //----------------------------------------------------------------------------------
      BeginDrawing();

      ClearBackground(BKGDCOLOR);

      for (uint8_t i = 0; i < CHIP8_SCREEN_HEIGHT; i++) {
        for (uint8_t j = 0; j < CHIP8_SCREEN_WIDTH; j++) {
          if (chip8_screen[i][j] != 0) {
            DrawRectangle(j*pixelWidth, i*pixelHeight, pixelWidth, pixelHeight, PIXELCOLOR);
          }
        }
      }

      EndDrawing();
      //----------------------------------------------------------------------------------
      if (chip8_sound > 0) {
        chip8_sound--;
      }
      if (chip8_delay > 0) {
        chip8_delay--;
      }
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow();        // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
