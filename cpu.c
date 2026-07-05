#include "cpu.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_ROM_SIZE (4096 - 0x200)
#define FONTSET_LOCATION 0x050

void cpuInit(CPU *cpu) {
  //Memory
  memset(cpu->V, 0, sizeof(cpu->V));
  cpu->idx = 0;
  cpu->pc = 0x200;

  //stack
  memset(cpu->stack, 0, sizeof(cpu->stack));
  cpu->sp = 0;

  //timers
  cpu->delayTimer = 0;
  cpu->soundTimer = 0;

  //input
  memset(cpu->keyboard, 0, sizeof(cpu->keyboard));

  //frameBuffer
  memset(cpu->frameBuffer, 0, sizeof(cpu->frameBuffer));

  //fontset
  memcpy(cpu->memory + FONTSET_LOCATION, fontSet, sizeof(fontSet));
  
  cpu->haultUntilPressed = false;
  cpu->waitReg = 0;
}

bool loadRom(const char *path, CPU *cpu) {
  FILE *rom;
  long fileSize;

  rom = fopen(path, "rb");
  if(!rom) {
    fprintf(stderr, "Rom failed to load\n");
    return false;
  }

  fseek(rom, 0, SEEK_END);
  fileSize = ftell(rom);
  rewind(rom);

  if(fileSize <= 0 || fileSize > MAX_ROM_SIZE) {
    fprintf(stderr, "Invalid ROM size\n");
    fclose(rom);
    return false;
  }

  size_t bytesRead = fread(cpu->memory + 0x200, 1, fileSize, rom);
  fclose(rom);

  //final check
  if(bytesRead != (size_t)fileSize) {
    fprintf(stderr, "ROM Read into Memory Failed\n");
    return false;
  }

  return true;
}

uint16_t fetchOpcode(CPU *cpu) {
  //Bounds check
  if(cpu->pc > 0xFFE) {
    fprintf(stderr, "Warning: Program counter out of bounds\n");
  }
  
  uint16_t opcode = cpu->memory[cpu->pc] << 8 | cpu->memory[cpu->pc + 1];
  printf("Opcode: %d\n", opcode);
  return opcode;
}

void programCycle(CPU *cpu) {
  //pause logic
  if(cpu->haultUntilPressed)
    return;

  uint16_t opcode = fetchOpcode(cpu);
  cpu->pc += 2;
  exeOpcode(cpu, opcode);
}


void exeOpcode(CPU *cpu, uint16_t opcode) {
  uint16_t chip8op = opcode & 0xF000;
  uint8_t x = getX(opcode);
  uint8_t y = getY(opcode);
  uint8_t kk = getKK(opcode);
  uint16_t nnn = getNNN(opcode);

  switch(chip8op) {
    case 0x0000:
      switch(opcode) {
        case 0x00E0:
          memset(cpu->frameBuffer, 0, sizeof(cpu->frameBuffer));
          break;
        case 0x00EE:
          cpu->pc = cpu->stack[--cpu->sp];
          break;
        default:
          break;
      }
      break;

    case 0x1000: {
      cpu->pc = nnn;
      break;
    }
    case 0x2000: {
      cpu->stack[cpu->sp++] = cpu->pc;
      cpu->pc = nnn;
      break;
    }
    case 0x3000: {
      if(cpu->V[x] == kk)
        cpu->pc += 2;
      break;
    }
    case 0x4000: {
      if(cpu->V[x] != kk)
        cpu->pc += 2;
      break;
    }
    case 0x5000: {
      if(cpu->V[x] == cpu->V[y])
        cpu->pc += 2;
      break;
    }
    case 0x6000: {
      cpu->V[x] = kk;
      break;
    }
    case 0x7000: {
      cpu->V[x] += kk;
      break;
    }
  }
} 
 


void updateTimers(CPU *cpu) {
  if(cpu->delayTimer > 0) --cpu->delayTimer;
}

inline uint16_t getNNN(uint16_t opcode) {
  uint16_t nnnval = (opcode & 0x0FFF);
  return nnnval;
}

inline uint8_t getKK(uint16_t opcode) {
  uint8_t nnval = (opcode & 0x00FF);
  return nnval;
}

inline uint8_t getX(uint16_t opcode) {
  uint8_t xval = (opcode & 0x0F00) >> 8; 
  return xval;
}

inline uint8_t getY(uint16_t opcode) {
  uint8_t yval = (opcode & 0x00F0) >> 4;
  return yval;
}















































