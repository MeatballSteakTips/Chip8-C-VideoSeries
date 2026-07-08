#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>
#include <stdbool.h>

#define NORMAL_RES (64 * 32)
#define HI_RES (128 * 64)


static const uint8_t fontSet[80] = {
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


typedef struct chip8cpu {
  //Registers & Memory
  uint8_t V[16];
  uint8_t memory[4096];
  uint16_t idx;
  uint16_t pc;

  //Stack 
  uint16_t stack[16];
  uint8_t sp;

  //inputs
  bool keyboard[16];

  //timers
  uint8_t delayTimer;
  uint8_t soundTimer;

  //Framebuffer
  uint8_t frameBuffer[NORMAL_RES];

  //Helper variables
  uint8_t waitReg;
  bool haultUntilPressed; 
}CPU;

typedef struct {
  uint16_t opType;
  uint8_t x;
  uint8_t y;
  uint8_t n;
  uint8_t kk;
  uint16_t nnn;
  uint8_t subType;
} cachedOp;

void cpuInit(CPU *cpu);
uint16_t fetchOpcode(CPU *cpu, uint16_t addr);
void programCycle(CPU *cpu);
void exeOpcode(CPU *cpu, cachedOp *op);
bool loadRom(const char *path, CPU *cpu);
void updateTimers(CPU *cpu);
void clearCache();
cachedOp decodeOpcode(uint16_t opcode);
#endif
