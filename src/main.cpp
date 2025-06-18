#include <iostream>
#include <stdint.h>
#include <chip8.h>

int main(void) {
  std::cout << "chip 8: start!" << '\n';
  for (uint8_t i = 0; i < CHIP8_BUFFER_HEIGHT; i++) {
    for (uint8_t j = 0; j < CHIP8_BUFFER_WIDTH; j++) {
      std::cout << std::to_string(CHIP8_BUFFER[i][j]) << ' ';
    }
    std::cout << '\n';
  }

  return 0;
}
