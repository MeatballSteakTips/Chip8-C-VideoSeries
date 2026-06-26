#include "cpu.h"
#include <string.h>

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
