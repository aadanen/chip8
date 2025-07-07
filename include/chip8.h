#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32
#define CHIP8_RAM_SIZE 4096
#define CHIP8_NUM_REGISTERS 16
#define CHIP8_STACK_DEPTH 16
#define CHIP8_NUM_QUIRKS 7

// for information about these quirks, go to database/quirks.json
enum {
  CHIP8_SHIFT,
  CHIP8_MEM_INCREMENT_X,
  CHIP8_MEM_I_UNCHANGED,
  CHIP8_WRAP,
  CHIP8_JUMP,
  CHIP8_VBLANK,
  CHIP8_VF_RESET
};


extern uint8_t chip8_screen[CHIP8_SCREEN_HEIGHT][CHIP8_SCREEN_WIDTH];
extern uint8_t chip8_delay;
extern uint8_t chip8_sound;
extern uint8_t chip8_ram[CHIP8_RAM_SIZE];

//extern uint8_t chip8_run_flag;
//extern uint8_t chip8_draw_flag;

void CHIP8_initialize(uint8_t* quirks);
bool CHIP8_load(char* rom_path);
void CHIP8_cycle(uint16_t keyboard, uint8_t ipf);
void CHIP8_dump();

#endif /* CHIP8_H */
