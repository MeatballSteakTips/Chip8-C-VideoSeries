#include "cpu.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_ROM_SIZE (4096 - 0x200)
#define FONTSET_LOCATION 0x050
#define CACHE_SIZE 4096 - 0x200
static cachedOp cache[CACHE_SIZE];
static bool cacheValid[CACHE_SIZE];

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

  clearCache();
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

uint16_t fetchOpcode(CPU *cpu, uint16_t addr) {
  //Bounds check
  if(cpu->pc > 0xFFE) {
    fprintf(stderr, "Warning: Program counter out of bounds\n");
  }
  
  uint16_t opcode = cpu->memory[addr] << 8 | cpu->memory[addr + 1];
  printf("Opcode: %d\n", opcode);
  return opcode;
}

cachedOp decodeOpcode(uint16_t opcode) {
  cachedOp op;

  op.opType = opcode & 0xF000;
  op.x   = (opcode & 0x0F00) >> 8;
  op.y   = (opcode & 0x00F0) >> 4;
  op.n   = opcode & 0x000F;
  op.kk  = opcode & 0x00FF;
  op.nnn = opcode & 0x0FFF;

  return op;
}

void programCycle(CPU *cpu) {
  //pause logic
  if(cpu->haultUntilPressed)
    return;

  uint16_t addr = cpu->pc;
  cpu->pc += 2;

  if(cacheValid[addr]) {
    exeOpcode(cpu, &cache[addr]);
  } else {
    uint16_t opcode = fetchOpcode(cpu, addr);

    cache[addr] = decodeOpcode(opcode);
    cacheValid[addr] = true;

    exeOpcode(cpu, &cache[addr]);
  }
}


void exeOpcode(CPU *cpu, cachedOp *op) {

  switch(op->opType) {
    case 0x0000:
      switch(op->kk) {
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
      cpu->pc = op->nnn;
      break;
    }
    case 0x2000: {
      cpu->stack[cpu->sp++] = cpu->pc;
      cpu->pc = op->nnn;
      break;
    }
    case 0x3000: {
      if(cpu->V[op->x] == op->kk)
        cpu->pc += 2;
      break;
    }
    case 0x4000: {
      if(cpu->V[op->x] != op->kk)
        cpu->pc += 2;
      break;
    }
    case 0x5000: {
      if(cpu->V[op->x] == cpu->V[op->y])
        cpu->pc += 2;
      break;
    }
    case 0x6000: {
      cpu->V[op->x] = op->kk;
      break;
    }
    case 0x7000: {
      cpu->V[op->x] += op->kk;
      break;
    }
  }
} 
 


void updateTimers(CPU *cpu) {
  if(cpu->delayTimer > 0) --cpu->delayTimer;
}

void clearCache() {
  memset(cacheValid, 0, sizeof(cacheValid));
}














































