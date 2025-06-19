#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <chip8.h>
#include <stdlib.h>

uint8_t chip8_ram[CHIP8_RAM_SIZE];
uint8_t chip8_screen[CHIP8_SCREEN_HEIGHT][CHIP8_SCREEN_WIDTH];
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

// INSTRUCTIONS //

// clear screen. turn all pixels in display buffer to 0
void CHIP8_00E0() {
  memset(chip8_screen, 0, sizeof(chip8_screen));
}

void CHIP8_00EE() {
}

// jump to NNN
void CHIP8_1NNN(uint16_t NNN) {
  chip8_pc = NNN;
}

// set VX to NN
void CHIP8_6XNN(uint8_t X, uint8_t NN) {
  chip8_v[X] = NN;
}

// add NN to VX
void CHIP8_7XNN(uint8_t X, uint8_t NN) {
  chip8_v[X] += NN;
}

// set chip8_index to NNN
void CHIP8_ANNN(uint16_t NNN) {
  chip8_index = NNN;
}

// display!
void CHIP8_DXYN(uint8_t X, uint8_t Y, uint8_t N) {
  uint8_t x = chip8_v[X] % CHIP8_SCREEN_WIDTH;
  uint8_t y = chip8_v[Y] % CHIP8_SCREEN_HEIGHT;
  chip8_v[0xf] = 0;

  // for N rows
  for (uint8_t i = 0; i < N; i++) {
    uint8_t row = chip8_ram[chip8_index+i];
    // for each bit in the row
    for (uint8_t offset = 7; offset >= 0; offset--) {
      uint8_t spritebit = (row >> offset) & 0x1;
      uint8_t pixel = chip8_screen[y + i][x + (7-offset)];
      if (spritebit && pixel) {
        chip8_screen[y + i][x + (7-offset)] = 0;
        chip8_v[0xf] = 1;
      } else if (spritebit && !pixel) {
        chip8_screen[y + i][x + (7-offset)] = 1;
      }
    }
   
    if (y + i == CHIP8_SCREEN_HEIGHT) {
      return;
    }
  }
}



// set up the CHIP8
// * loads font into RAM
void CHIP8_initialize() {
  // load the font into memory
  memcpy(chip8_ram+80, FONT_DATA, 5*16*sizeof(uint8_t));
}


void CHIP8_load(char* rom_path) {
  return;
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
  switch (opcode) {
    case 0x0:
      if (NN == 0xE0) {
        CHIP8_00E0();
      } else {
        CHIP8_00EE();
      }
      break;
    case 0x1:
      CHIP8_1NNN(NNN);
      break;
    case 0x6:
      CHIP8_6XNN(X, NN);
      break;
    case 0x7:
      CHIP8_7XNN(X, NN);
      break;
    case 0xA:
      CHIP8_ANNN(NNN);
      break;
    case 0xD:
      CHIP8_DXYN(X, Y, N);
      break;
  }
}





