#include <stdint.h>
#include <string.h>
#include <chip8.h>
#include <stdlib.h>

uint8_t chip8_ram[chip8_ram_SIZE];
uint8_t chip8_screen[CHIP8_SCREEN_HEIGHT/8][CHIP8_SCREEN_WIDTH/8];
uint8_t chip8_delay = 0;
uint8_t chip8_sound = 0;
uint8_t chip8_v[CHIP8_NUM_REGISTERS];
uint16_t chip8_pc = 0x200;
uint16_t chip8_index = 0x200;

uint16_t chip8_stack[CHIP8_STACK_DEPTH];
uint8_t chip8_stack_top = 0;

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


// set up the CHIP8
// * loads font into RAM
void CHIP8_initialize() {
  // load the font into memory
  memcpy(chip8_ram+80, FONT_DATA, 5*16*sizeof(uint8_t));
}


void CHIP8_load(char* rom_path) {
}


void CHIP8_cycle() {
}


uint16_t CHIP8_pop() {
  uint16_t result = chip8_stack[chip8_stack_top];
  if (chip8_stack_top > 0) {
    chip8_stack_top--;
  }
  return result;
}


void CHIP8_push(uint16_t data) {
  chip8_stack_top++;
  if (chip8_stack_top > CHIP8_STACK_DEPTH) {
    printf("stack overflow!!!");
    abort();
  }
  chip8_stack[chip8_stack_top] = data;
}


// get the instruction at the location in memory stored in the PC
uint16_t CHIP8_fetch() {
  uint16_t instruction = 
    ((uint16_t)chip8_ram[chip8_pc] << 8) | (uint16_t)chip8_ram[chip8_pc+1];
  chip8_pc += 2;
  return instruction;
}

void CHIP8_cycle() {
  uint16_t instruction = CHIP8_fetch();

  // this is wasting work because not every instruction will use these values
  // but i am kinda unconcerned with performance and it will make programming
  // easier
  uint8_t opcode = (uint8_t)(instruction >> 12);
  uint8_t X = (uint8_t)((instruction >> 8) & 0x0f);
  uint8_t Y = (uint8_t)((instruction >> 4) & 0x00f);
  uint8_t N = (uint8_t)(instruction & 0x000f);
  uint8_t NN = (uint8_t)(instruction & 0x00ff);
  uint16_t NNN = instruction & 0x0fff;
}

