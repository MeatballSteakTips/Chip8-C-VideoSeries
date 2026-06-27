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
  memcpy(cpu->memory + FONTSET_LOCATION, fontSet, 80);
  
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

void programCycle(CPU *cpu) {

}

uint16_t fetchOpcode(CPU *cpu) {

}

void exeOpcode(CPU *cpu, uint16_t opcodes) {

}

void updateTimers(CPU *cpu) {

}
