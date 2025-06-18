#include <iostream>
#include <stdint.h>
#include <cstring>


#include <chip8.h>
#include <raylib.h>

uint32_t initialize() {
  // load the font into memory
  uint8_t FONT_DATA[5*16] = {
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
  memcpy(CHIP8_RAM+80, FONT_DATA, 5*16*sizeof(uint8_t));

  return 0;
}

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

int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------
  uint32_t screenWidth = 1600;
  uint32_t screenHeight = 800;
  uint32_t pixelHeight = screenHeight/CHIP8_SCREEN_HEIGHT;
  uint32_t pixelWidth = screenWidth/CHIP8_SCREEN_WIDTH;
  uint32_t key = 0;
  SetTraceLogLevel(LOG_ERROR);

  initialize();
  InitWindow(screenWidth, screenHeight, "CHIP8");

  SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose())    // Detect window close button or ESC key
  {
      // Update
      //----------------------------------------------------------------------------------
      // TODO: Update your variables here
      //----------------------------------------------------------------------------------
      key = normalize_input(GetKeyPressed());
      std::cout << key << '\n';

      // Draw
      //----------------------------------------------------------------------------------
      BeginDrawing();

      ClearBackground(BLACK);

      /*
      // loop over the CHIP8 buffer
      for (uint8_t i = 0; i < CHIP8_BUFFER_HEIGHT; i++) {
        for (uint8_t j = 0; j < CHIP8_BUFFER_WIDTH; j++) {
          if (CHIP8_BUFFER[i][j] != 0) {
            DrawRectangle(i*pixelHeight, j*pixelWidth, pixelWidth, pixelHeight, RAYWHITE);
          }
        }
      }
      */


      EndDrawing();
      //----------------------------------------------------------------------------------
      /*
      screenWidth = GetScreenWidth();
      screenHeight = GetScreenHeight();
      pixelHeight = screenHeight/CHIP8_SCREEN_HEIGHT;
      pixelWidth = screenWidth/CHIP8_SCREEN_WIDTH;
      */
      if (CHIP8_DELAY > 0) {
        CHIP8_DELAY--;
      }
      if (CHIP8_SOUND > 0) {
        CHIP8_SOUND--;
      }
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow();        // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
