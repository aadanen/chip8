#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <chip8.h>
#include <stdlib.h>
#include <time.h>

uint8_t chip8_ram[CHIP8_RAM_SIZE];
uint8_t chip8_screen[CHIP8_SCREEN_HEIGHT][CHIP8_SCREEN_WIDTH];
uint8_t chip8_delay = 0;
uint8_t chip8_sound = 0;
uint8_t chip8_v[CHIP8_NUM_REGISTERS];
uint16_t chip8_kb = 0;
uint16_t chip8_pc = 0x200;
uint16_t chip8_index = 0x200;
uint8_t chip8_run_flag = 0;
uint8_t chip8_draw_flag = 0;

uint16_t chip8_stack[CHIP8_STACK_DEPTH];
uint8_t chip8_stack_top = 0;

// quirks
uint8_t chip8_quirks[7];
enum CHIP8_QUIRK_INDEX quirk_index;

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

uint16_t CHIP8_pop();
void CHIP8_push(uint16_t);

// INSTRUCTIONS //

// clear screen. turn all pixels in display buffer to 0
void CHIP8_00E0() {
  memset(chip8_screen, 0, sizeof(chip8_screen));
  //chip8_draw_flag = 1;
}

// return to the address at the top of the stack
void CHIP8_00EE() {
  chip8_pc = CHIP8_pop();
}

// jump to NNN
void CHIP8_1NNN(uint16_t NNN) {
  chip8_pc = NNN;
}

// push current location to the stack and jump to NNN
void CHIP8_2NNN(uint16_t NNN) {
  CHIP8_push(chip8_pc);
  chip8_pc = NNN;
}

// skip the next instruction if vX == NN
void CHIP8_3XNN(uint8_t X, uint8_t NN) {
  if (chip8_v[X] == NN) {
    chip8_pc += 2;
  }
}

// skip the next instruction if vX != NN
void CHIP8_4XNN(uint8_t X, uint8_t NN) {
  if (chip8_v[X] != NN) {
    chip8_pc += 2;
  }
}

// skip the next instruction if vX == vY
void CHIP8_5XY0(uint8_t X, uint8_t Y) {
  if (chip8_v[X] == chip8_v[Y]) {
    chip8_pc += 2;
  }
}


// set VX to NN
void CHIP8_6XNN(uint8_t X, uint8_t NN) {
  chip8_v[X] = NN;
}

// add NN to VX
void CHIP8_7XNN(uint8_t X, uint8_t NN) {
  chip8_v[X] += NN;
}


void CHIP8_8XY0(uint8_t X, uint8_t Y) {
  chip8_v[X] = chip8_v[Y];
}
void CHIP8_8XY1(uint8_t X, uint8_t Y) {
  chip8_v[X] = chip8_v[X] | chip8_v[Y];
  chip8_v[0xf] = 0;
}
void CHIP8_8XY2(uint8_t X, uint8_t Y) {
  chip8_v[X] = chip8_v[X] & chip8_v[Y];
  chip8_v[0xf] = 0;
}
void CHIP8_8XY3(uint8_t X, uint8_t Y) {
  chip8_v[X] = chip8_v[X] ^ chip8_v[Y];
  chip8_v[0xf] = 0;
}
void CHIP8_8XY4(uint8_t X, uint8_t Y) {
  if (chip8_v[Y] > 255 - chip8_v[X]) {
    chip8_v[X] += chip8_v[Y];
    chip8_v[0xf] = 1;
  } else {
    chip8_v[X] += chip8_v[Y];
    chip8_v[0xf] = 0;
  }
}
void CHIP8_8XY5(uint8_t X, uint8_t Y) {
  if (chip8_v[X] >= chip8_v[Y]) {
    chip8_v[X] -= chip8_v[Y];
    chip8_v[0xf] = 1;
  } else {
    chip8_v[X] -= chip8_v[Y];
    chip8_v[0xf] = 0;
  }
}

void CHIP8_8XY6(uint8_t X, uint8_t Y) {
  quirk_index = SHIFT;
  if (!chip8_quirks[quirk_index]) {
    chip8_v[X] = chip8_v[Y];
  }
  if (chip8_v[X] & 0x1) {
    chip8_v[X] >>= 1;
    chip8_v[0xf] = 1;
  } else {
    chip8_v[X] >>= 1;
    chip8_v[0xf] = 0;
  }
}
void CHIP8_8XY7(uint8_t X, uint8_t Y) {
  if (chip8_v[Y] >= chip8_v[X]) {
    chip8_v[X] = chip8_v[Y] - chip8_v[X];
    chip8_v[0xf] = 1;
  } else {
    chip8_v[X] = chip8_v[Y] - chip8_v[X];
    chip8_v[0xf] = 0;
  }
}
void CHIP8_8XYE(uint8_t X, uint8_t Y) {
  quirk_index = SHIFT;
  if (!chip8_quirks[quirk_index]) {
    chip8_v[X] = chip8_v[Y];
  }
  if (chip8_v[X] & 0x80) {
    chip8_v[X] <<= 1;
    chip8_v[0xf] = 1;
  } else {
    chip8_v[X] <<= 1;
    chip8_v[0xf] = 0;
  }
}


// skip the next instruction if vX != vY
void CHIP8_9XY0(uint8_t X, uint8_t Y) {
  if (chip8_v[X] != chip8_v[Y]) {
    chip8_pc += 2;
  }
}

// set chip8_index to NNN
void CHIP8_ANNN(uint16_t NNN) {
  chip8_index = NNN;
}

void CHIP8_BNNN(uint8_t X, uint16_t NNN) {
  quirk_index = JUMP;
  if (chip8_quirks[quirk_index]) {
    chip8_pc = chip8_v[X]+NNN;
  } else {
    chip8_pc = chip8_v[0]+NNN;
  }
}

void CHIP8_CXNN(uint8_t X, uint8_t NN) {
  int r = rand();
  chip8_v[X] = (uint8_t)(r & (int)NN);
}

// display!
void CHIP8_DXYN(uint8_t X, uint8_t Y, uint8_t N) {
  uint8_t x = chip8_v[X] % CHIP8_SCREEN_WIDTH;
  uint8_t y = chip8_v[Y] % CHIP8_SCREEN_HEIGHT;
  chip8_v[0xf] = 0;
  quirk_index = WRAP;

  // for N rows
  for (uint8_t i = 0; i < N; i++) {
    if (y + i == CHIP8_SCREEN_HEIGHT) {
      return;
    }

    uint8_t row = chip8_ram[chip8_index+i];
    // for each bit in the row
    for (uint8_t j = 0; j < 8; j++) {
      if (!chip8_quirks[quirk_index] && (x + j == CHIP8_SCREEN_WIDTH)) {
        break;
      }
      uint8_t spritebit = row & (0x80 >> j);
      uint8_t* pixel = &(chip8_screen[y + i][(x + j) % CHIP8_SCREEN_WIDTH]);
      if (spritebit && *pixel) {
        *pixel = 0;
        chip8_v[0xf] = 1;
      } else if (spritebit && !*pixel) {
        *pixel = 1;
      }
    }
  }
}

void CHIP8_EX9E(uint8_t X) {
  if ((chip8_kb >> chip8_v[X]) & 0x1) {
    chip8_pc += 2;
  }
}
void CHIP8_EXA1(uint8_t X) {
  if (!((chip8_kb >> chip8_v[X]) & 0x1)) {
    chip8_pc += 2;
  }
}



void CHIP8_FX07(uint8_t X) {
  chip8_v[X] = chip8_delay;
}
void CHIP8_FX15(uint8_t X) {
  chip8_delay = chip8_v[X];
}
void CHIP8_FX18(uint8_t X) {
  chip8_sound = chip8_v[X];
}
// spaceflight 2091 behavior
void CHIP8_FX1E(uint8_t X) {
  chip8_index += chip8_v[X];
  chip8_v[0xf] = chip8_index > CHIP8_RAM_SIZE;
}

// trigger when key releases
// if keyboard and chip8_kb match, nothing changed, loop again
// if any bit went from 1 to 0, put that in VX
void CHIP8_FX0A(uint8_t X, uint16_t keyboard) {
  for (uint8_t i = 0; i < 0xf; i++) {
    // if a key was pressed and now its not
    if (((chip8_kb >> i) & 0x1) > ((keyboard >> i) & 0x1)) {
      chip8_v[X] = i;
      return;
    }
  }
  chip8_pc -= 2;
}
void CHIP8_FX29(uint8_t X) {
  // font starts at chip8_ram[80]
  // each letter is 5 bytes
  chip8_index = (uint16_t)(80+(5*(X & 0xf)));
}

// example: if vx = 156, put 1, 5, and 6 at index, index+1, index+2
void CHIP8_FX33(uint8_t X) {
  chip8_ram[chip8_index] = (chip8_v[X]/100) % 10;
  chip8_ram[chip8_index+1] = (chip8_v[X]/10) % 10;
  chip8_ram[chip8_index+2] = chip8_v[X] % 10;
}
void CHIP8_FX55(uint8_t X) {
  quirk_index = MEM_I_UNCHANGED;
  if (!chip8_quirks[MEM_I_UNCHANGED]) {
    for (int i = 0; i <= X; i++) {
      chip8_ram[chip8_index] = chip8_v[i];
      chip8_index++;
    }
    quirk_index = MEM_INCREMENT_X;
    if (chip8_quirks[quirk_index]) {
      chip8_index--;
    }
  } else {
    for (int i = 0; i <= X; i++) {
      chip8_ram[chip8_index] = chip8_v[i];
    }
  }
}
void CHIP8_FX65(uint8_t X) {
  for (int i = 0; i <= X; i++) {
    chip8_v[i] = chip8_ram[chip8_index];
    chip8_index++;
  }
  if (chip8_quirks[quirk_index]) {
    chip8_index--;
  }
}

// set up the CHIP8
// * loads font into RAM
void CHIP8_initialize(uint8_t* quirks) {
  // load the font into memory
  memcpy(chip8_ram+80, FONT_DATA, 5*16*sizeof(uint8_t));
  memcpy(chip8_quirks, quirks, CHIP8_NUM_QUIRKS);
  srand(time(NULL));
}


void CHIP8_load(char* rom_path) {
  FILE* fptr = fopen(rom_path, "rb");
  if (!fptr) {
    printf("failed to open rom file\n");
    abort();
  }
  printf("reading rom\n");

  int32_t buf;
  uint8_t* ramptr = chip8_ram + 0x200;
  while ((buf = fgetc(fptr)) != EOF) {
    *ramptr = (uint8_t)buf;
    ramptr++;
  }
  printf("done\n");
  fclose(fptr);
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

void CHIP8_cycle(uint16_t keyboard, uint8_t ipf) {
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
    case 0x2:
      CHIP8_2NNN(NNN);
      break;
    case 0x3:
      CHIP8_3XNN(X, NN);
      break;
    case 0x4:
      CHIP8_4XNN(X, NN);
      break;
    case 0x5:
      CHIP8_5XY0(X, Y);
      break;
    case 0x6:
      CHIP8_6XNN(X, NN);
      break;
    case 0x7:
      CHIP8_7XNN(X, NN);
      break;
    case 0x8:
      switch (N) {
        case 0x0:
          CHIP8_8XY0(X, Y);
          break;
        case 0x1:
          CHIP8_8XY1(X, Y);
          break;
        case 0x2:
          CHIP8_8XY2(X, Y);
          break;
        case 0x3:
          CHIP8_8XY3(X, Y);
          break;
        case 0x4:
          CHIP8_8XY4(X, Y);
          break;
        case 0x5:
          CHIP8_8XY5(X, Y);
          break;
        case 0x6:
          CHIP8_8XY6(X, Y);
          break;
        case 0x7:
          CHIP8_8XY7(X, Y);
          break;
        case 0xE:
          CHIP8_8XYE(X, Y);
          break;
      }
      break;
    case 0x9:
      CHIP8_9XY0(X, Y);
      break;
    case 0xA:
      CHIP8_ANNN(NNN);
      break;
    case 0xB:
      CHIP8_BNNN(X, NNN);
      break;
    case 0xC:
      CHIP8_CXNN(X, NN);
      break;
    case 0xD:
      quirk_index = VBLANK;
      if (chip8_quirks[quirk_index]) {
        if (ipf == 0) {
          CHIP8_DXYN(X, Y, N);
        } else {
          // no-op and wait for vblank
          chip8_pc -= 2;
        }
        
      } else {
        CHIP8_DXYN(X, Y, N);
      }
      break;
    case 0xE:
      if (NN == 0x9E) {
        CHIP8_EX9E(X);
      } else {
        CHIP8_EXA1(X);
      }
      break;
    case 0xF:
      switch (NN) {
        case 0x07:
          CHIP8_FX07(X);
          break;
        case 0x15:
          CHIP8_FX15(X);
          break;
        case 0x18:
          CHIP8_FX18(X);
          break;
        case 0x1E:
          CHIP8_FX1E(X);
          break;
        case 0x0A:
          CHIP8_FX0A(X, keyboard);
          break;
        case 0x29:
          CHIP8_FX29(X);
          break;
        case 0x33:
          CHIP8_FX33(X);
          break;
        case 0x55:
          CHIP8_FX55(X);
          break;
        case 0x65:
          CHIP8_FX65(X);
          break;
      }
  }
  chip8_kb = keyboard;
}

void CHIP8_dump() {
  uint16_t instruction = 
    ((uint16_t)chip8_ram[chip8_pc] << 8) | (uint16_t)chip8_ram[chip8_pc+1];
  printf("%04x\n",instruction);
  for (uint8_t i = 0; i <= 0xf; i++) {
    printf("V%X = %02x\n", i, chip8_v[i]);
  }
}





