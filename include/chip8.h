#include <stdint.h>
#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32
#define CHIP8_RAM_SIZE 4096
#define CHIP8_NUM_REGISTERS 16
#define CHIP8_STACK_DEPTH 16
#define CHIP8_CLOCK_LENGTH 1.0/700.0

extern uint8_t chip8_screen[CHIP8_SCREEN_HEIGHT/8][CHIP8_SCREEN_WIDTH/8];
extern uint8_t chip8_delay;
extern uint8_t chip8_sound;

void CHIP8_initialize();
void CHIP8_load(char* rom_path);
void CHIP8_cycle();


