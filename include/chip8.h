#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32
#define CHIP8_RAM_SIZE 4096
#define CHIP8_NUM_REGISTERS 16
#define CHIP8_STACK_DEPTH 16
#define CHIP8_CLOCK_SPEED 700
#define CHIP8_OLD_SHIFT 1
#define CHIP8_OLD_JUMP_OFFSET 1

extern uint8_t chip8_screen[CHIP8_SCREEN_HEIGHT][CHIP8_SCREEN_WIDTH];
extern uint8_t chip8_delay;
extern uint8_t chip8_sound;
extern uint8_t chip8_ram[CHIP8_RAM_SIZE];

//extern uint8_t chip8_run_flag;
//extern uint8_t chip8_draw_flag;

void CHIP8_initialize();
void CHIP8_load(char* rom_path);
void CHIP8_cycle();

#endif /* CHIP8_H */
