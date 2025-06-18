#include <iostream>
#include <stdint.h>
#include <chip8.h>
#include <raylib.h>

int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------
  uint32_t screenWidth = 1600;
  uint32_t screenHeight = 800;
  uint32_t pixelHeight = screenHeight/CHIP8_BUFFER_HEIGHT;
  uint32_t pixelWidth = screenWidth/CHIP8_BUFFER_WIDTH;

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

      // Draw
      //----------------------------------------------------------------------------------
      BeginDrawing();

      ClearBackground(BLACK);

      // loop over the CHIP8 buffer
      for (uint8_t i = 0; i < CHIP8_BUFFER_HEIGHT; i++) {
        for (uint8_t j = 0; j < CHIP8_BUFFER_WIDTH; j++) {
          if (CHIP8_BUFFER[i][j] != 0) {
            DrawRectangle(i*pixelHeight, j*pixelWidth, pixelWidth, pixelHeight, RAYWHITE);
          }
        }
      }


      EndDrawing();
      //----------------------------------------------------------------------------------
      screenWidth = GetScreenWidth();
      screenHeight = GetScreenHeight();
      pixelHeight = screenHeight/CHIP8_BUFFER_HEIGHT;
      pixelWidth = screenWidth/CHIP8_BUFFER_WIDTH;
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow();        // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
