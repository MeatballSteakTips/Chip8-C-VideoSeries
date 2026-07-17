#include "cpu.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_ROM_SIZE (4096 - 0x200)
#define FONTSET_LOCATION 0x050
#define CACHE_SIZE 4096 - 0x200
#define FLAG_REGISTER 0x0F
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
    case 0x8000: {
      switch(op->n) {
        case 0x0000: cpu->V[op->x] = cpu->V[op->y]; break;
        case 0x0001: cpu->V[op->x] |= cpu->V[op->y]; break;
        case 0x0002: cpu->V[op->x] &= cpu->V[op->y]; break;
        case 0x0003: cpu->V[op->x] ^= cpu->V[op->y]; break;
        case 0x0004: {
          uint16_t sum = cpu->V[op->x] + cpu->V[op->y];
          cpu->V[op->x] = sum & 0xFF;
          cpu->V[FLAG_REGISTER] = (sum > 0xFF) ? 1 : 0;
          break;
        }
        case 0x0005: {
          uint8_t vxOld = cpu->V[op->x];
          uint16_t diff = vxOld - cpu->V[op->y];
          cpu->V[op->x] = diff & 0xFF;
          cpu->V[FLAG_REGISTER] = (vxOld >= cpu->V[op->y]) ? 1 : 0;
          break;            
        }
        case 0x0006: {
          uint8_t lsb = cpu->V[op->x] & 0x01;
          cpu->V[op->x] >>= 1;
          cpu->V[FLAG_REGISTER] = lsb;
          break;
        }
        case 0x0007: {
          uint8_t vxOld = cpu->V[op->x];
          cpu->V[op->x] = cpu->V[op->y] - vxOld;
          cpu->V[FLAG_REGISTER] = (cpu->V[op->y] >= vxOld) ? 1 : 0;
          break;
        }
        case 0x000E: {
          uint8_t msb = (cpu->V[op->x] & 0x80) >> 7;
          cpu->V[op->x] <<= 1;
          cpu->V[FLAG_REGISTER] = msb;
          break;
        }
      }
      break;
    }
    case 0x9000: {
      if(cpu->V[op->x] != cpu->V[op->y])
        cpu->pc += 2;
      break;
    }
    case 0xA000: {
      cpu->idx = op->nnn;
      break;
    }
    case 0xB000: {
      cpu->pc = cpu->V[0] + op->nnn;
      break;
    }
    case 0xC000: {
      cpu->V[op->x] = randomNumber() & op->nnn;
      break;
    }
    case 0xD000: {
      drawSprite(cpu, op->x, op->y, op->n);
      break;
    }
    case 0xE000: {
      switch(op->kk) {
        case 0x9E:
          if(cpu->keyboard[cpu->V[op->x] & 0xF])
            cpu->pc += 2;
          break;
        case 0xA1: 
          if(!cpu->keyboard[cpu->V[op->x] & 0xF])
            cpu->pc += 2;
          break;
      }
      break;
    }
    case 0xF000: {
      switch(op->kk) {
        case 0x07:
          cpu->V[op->x] = cpu->delayTimer;
          break;
        case 0x0A:
          cpu->haultUntilPressed = true;
          cpu->waitReg = op->x;
          break;
        case 0x15:
          cpu->delayTimer = cpu->V[op->x];
          break;
        case 0x18:
          cpu->soundTimer = cpu->V[op->x];
          break;
        case 0x1E:
          cpu->idx += cpu->V[op->x];
          break;
        case 0x29:
          cpu->idx = FONTSET_LOCATION + (cpu->V[op->x] * 5);
          break;
        case 0x33: {
          uint8_t val = cpu->V[op->x];
          cpu->memory[cpu->idx] = val / 100;
          cpu->memory[cpu->idx + 1] = (val / 10) % 10;
          cpu->memory[cpu->idx + 2] = val % 10;
          break;
        }
        case 0x55: {
          size_t n = (size_t)(op->x + 1);
          size_t avail = 0x1000 - (size_t)cpu->idx;
          if(n > avail)
            n = avail;
          memcpy(&cpu->memory[cpu->idx], &cpu->V[0], n);
          cpu->idx += (op->x + 1);
          break;
        }
        case 0x65: {
          size_t n = (size_t)(op->x + 1);
          size_t avail = 0x1000 - (size_t)cpu->idx;
          if(n > avail)
            n = avail;
          memcpy(&cpu->V[0], &cpu->memory[cpu->idx], n);
          cpu->idx += (op->x + 1);
          break;
        }
      } 
      break;
    }
  }
} 
 
uint8_t randomNumber() {
  uint8_t gen = rand() % 256;
  return gen;
}

void updateTimers(CPU *cpu) {
  if(cpu->delayTimer > 0) --cpu->delayTimer;
}

void clearCache() {
  memset(cacheValid, 0, sizeof(cacheValid));
}

void drawSprite(CPU *cpu, uint8_t x, uint8_t y, uint8_t h) {
  uint8_t xPos = cpu->V[x] % 64;
  uint8_t yPos = cpu->V[y] % 32;

  cpu->V[FLAG_REGISTER] = 0;

  for(uint8_t row = 0; row < h; row++) {
    uint16_t spriteByte = cpu->memory[cpu->idx + row];

    for(uint8_t col = 0; col < 8; col++) {
      uint8_t spritePixel = (spriteByte >> (7 - col)) & 0x1;

      if(spritePixel) {
        uint8_t px = (xPos + col) % 64;
        uint8_t py = (yPos + row) % 32;
        uint16_t fbIdx = py * 64 + px; //index = row * num_cols + col

        if(cpu->frameBuffer[fbIdx] == 1) {
          cpu->V[FLAG_REGISTER] = 1;
        }

        cpu->frameBuffer[fbIdx] ^= 1;
      }
    }
  }
}











































