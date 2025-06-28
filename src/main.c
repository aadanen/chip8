#include <stdint.h>
#include <stdio.h>
#include <raylib.h>
#include <chip8.h>

#define PIXELCOLOR RAYWHITE
#define BKGDCOLOR BLACK

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
  uint16_t keyboard;
  const uint32_t target_fps = 60;
  const uint32_t cycles_per_frame = CHIP8_CLOCK_SPEED/target_fps;


  SetTraceLogLevel(LOG_ERROR);

  InitWindow(screenWidth, screenHeight, "CHIP8");
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(4096);
  SetTargetFPS(target_fps); 

  Sound beep = LoadSound("src/261.wav");

  CHIP8_initialize();
  CHIP8_load(argv[1]);

  // Main game loop
  while (!WindowShouldClose()) {

    keyboard = 0;
    if (IsKeyDown(49))
      keyboard |= 0x2;
    if (IsKeyDown(50))
      keyboard |= 0x4;
    if (IsKeyDown(51))
      keyboard |= 0x8;
    if (IsKeyDown(52))
      keyboard |= 0x1000;

    if (IsKeyDown(81))
      keyboard |= 0x10;
    if (IsKeyDown(87))
      keyboard |= 0x20;
    if (IsKeyDown(69))
      keyboard |= 0x40;
    if (IsKeyDown(82))
      keyboard |= 0x2000;

    if (IsKeyDown(65))
      keyboard |= 0x80;
    if (IsKeyDown(83))
      keyboard |= 0x100;
    if (IsKeyDown(68))
      keyboard |= 0x200;
    if (IsKeyDown(70))
      keyboard |= 0x4000;

    if (IsKeyDown(90))
      keyboard |= 0x400;
    if (IsKeyDown(88))
      keyboard |= 0x1;
    if (IsKeyDown(67))
      keyboard |= 0x800;
    if (IsKeyDown(86))
      keyboard |= 0x8000;

    // 1 frame will have roughly 11 clock cycles
    // maybe later i can run the two processes seperately
    // like one thread is the screen and another thread is the emulator
    for (uint32_t i = 0; i < cycles_per_frame; i++) {
      CHIP8_cycle(keyboard);
    }

    // sound
    if (chip8_sound > 0 && !IsSoundPlaying(beep)) {
      PlaySound(beep);
    }
    if (chip8_sound == 0 && IsSoundPlaying(beep)) {
      StopSound(beep);
    }

    // Draw
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
