#include "cpu.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  if(argc == 1 || argc >= 3) {
    fprintf(stderr, "Usage: chip8 \"FilePath\"\n");
    return -1;
  }

  char pathBuffer[256];
  strncpy(pathBuffer, argv[1], sizeof(pathBuffer) - 1);
  pathBuffer[sizeof(pathBuffer) - 1] = '\0';

  CPU cpu;
  cpuInit(&cpu);

  if(!loadRom(pathBuffer, &cpu)) {
    fprintf(stderr, "Error: Failed to load Rom\n");
    return -1;
  }

  return 0;
}
