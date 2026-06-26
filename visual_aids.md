# Chip8 Emulator — Visual Aids

Text-based diagrams for use in the video series. Each block can be
copy-pasted into a slide, terminal, or code editor on screen.

---

## 1. System Overview 

The whole emulator at a glance. The CPU sits in the middle and
talks to memory, input, display, and timers.

```
   +-----------+                +-----------+
   |  Keypad   |                |  Display  |
   |  16 keys  |                |  64 x 32  |
   +-----+-----+                +-----+-----+
         |  (input)                ^  (pixels)
         v                         |
   +---------------------------------------------+
   |                    CPU                      |
   |                                             |
   |   +-----------------+  +-----------------+  |
   |   |    Registers    |  |     Stack       |  |
   |   | V0..VF, I, PC   |  |  16 x 16-bit    |  |
   |   +-----------------+  +-----------------+  |
   |   +-----------------+  +-----------------+  |
   |   |     Timers      |  |    SP, DT, ST   |  |
   |   +-----------------+  +-----------------+  |
   +-----------------+-----+-----+---------------+
                     |           |
            (opcodes)|           |(read/write)
                     v           v
                +---------------------------+
                |         Memory            |
                |       4096 bytes          |
                +---------------------------+
```

---

## 2. Registers

### 2a. General purpose registers (8-bit each)

There are 16 of them, named V0 through VF. VF is special:
it's used as a "carry / borrow / collision" flag by some
instructions (8XY1..8XYE, FX55/FX65, DXYN).

```
   +-----+-----+-----+-----+-----+-----+-----+-----+
   | V0  | V1  | V2  | V3  | V4  | V5  | V6  | V7  |
   +-----+-----+-----+-----+-----+-----+-----+-----+
   | V8  | V9  | VA  | VB  | VC  | VD  | VE  | VF  |
   +-----+-----+-----+-----+-----+-----+-----+-----+
   <--- 1 byte each (0..255) --->          ^
                                        carry / flag
```

### 2b. Special registers

```
   +------+----------+--------------------------------------+
   | Name | Size     | Purpose                              |
   +------+----------+--------------------------------------+
   |  I   | 16 bits  | Address pointer into memory          |
   |      |          | (used by sprite, store, load ops)    |
   +------+----------+--------------------------------------+
   |  PC  | 16 bits  | Program Counter.                     |
   |      |          | Points to the next opcode to fetch.  |
   |      |          | Starts at 0x200.                     |
   +------+----------+--------------------------------------+
   |  SP  |  8 bits  | Stack Pointer. Index 0..15 into the  |
   |      |          | stack. Starts at -1 (empty).         |
   +------+----------+--------------------------------------+
   |  DT  |  8 bits  | Delay Timer. Counts at 60 Hz.        |
   |      |          | Decrements every frame until 0.     |
   +------+----------+--------------------------------------+
   |  ST  |  8 bits  | Sound Timer. Counts at 60 Hz.        |
   |      |          | Beeps while non-zero.               |
   +------+----------+--------------------------------------+
```

---

## 3. Memory Map (4096 bytes)

The address space is small enough to draw in full. Everything
from 0x200 onwards is "yours" — that's where the ROM goes.

```
   0x000  +--------------------------------------+
          |   Reserved (used by interpreter)     |
          |                                      |
   0x050  |  +--------------------------------+  |
          |  |  Built-in fontset              |  |
          |  |  16 chars x 5 bytes = 80 bytes |  |
          |  |  "0".."9", "A".."F"            |  |
   0x0A0  |  +--------------------------------+  |
          |  |  (free)                         |  |
   0x1FF  +--------------------------------------+
   0x200  |  Program ROM / data starts here      |  <-- PC reset value
          |  +--------------------------------+  |
          |  |  Game instructions & data      |  |
          |  |  (max 3584 bytes)              |  |
          |  |                                |  |
          |  |                                |  |
   0xFFF  |  +--------------------------------+  |
          +--------------------------------------+
```

---

## 4. Stack

A simple array of 16 entries, each holding a 16-bit return
address. SP points at the *top* (most recent push).

```
       index   value
      +-----+----------+
 top  | 15  |  0x0ABC  |  <-- SP (most recent return address)
      +-----+----------+
      | 14  |  0x021A  |
      +-----+----------+
      | 13  |  0x0000  |
      +-----+----------+
      | ... |  .....   |
      +-----+----------+
      |  1  |  0x0000  |
      +-----+----------+
 bot  |  0  |  0x0000  |
      +-----+----------+

   * SP starts at -1 (empty stack)
   * CALL  ->  SP++;  stack[SP] = PC;  PC = NNN
   * RET   ->  PC = stack[SP];  SP--;
   * Max nesting depth = 16 (programs can corrupt this)
```

---

## 5. Display (64 x 32, monochrome)

Each cell = 1 pixel. A pixel is either ON (1) or OFF (0).
Total = 64 * 32 = 2048 bits = 256 bytes.

```
   +----------------------------------------------------------------+
   |                                                                |
   |          0              32              63                     |
   |          |               |               |                      |
   |     0  +-+---------------+---------------+-+                   |
   |        | . . . . . . . . . . . . . . . . . |                   |
   |        | . . . . . . . . . . . . . . . . . |                   |
   |        | . . . . . . . . . . . . . . . . . |                   |
   |        | . . . . . . . . . . . . . . . . . |                   |
   |        | . . . . . . . . . . . . . . . . . |                   |
   |   31  +-+---------------+---------------+-+                   |
   |                                                                |
   +----------------------------------------------------------------+

   * Stored as 64 uint64s, 32 bytes, 2048 bools, etc.
   * DXYN draws an N-byte tall sprite from memory[I] at (VX, VY).
   * Pixels are XOR'd — collision sets VF = 1.
```

---

## 6. Keypad (16 keys, hex layout)

The original layout is 4x4. Each key is referenced by a hex
value 0x0..0xF.

```
   +-----+-----+-----+-----+
   |  1  |  2  |  3  |  C  |     Logical (and modern) key layout
   +-----+-----+-----+-----+
   |  4  |  5  |  6  |  D  |
   +-----+-----+-----+-----+
   |  7  |  8  |  9  |  E  |
   +-----+-----+-----+-----+
   |  A  |  0  |  B  |  F  |
   +-----+-----+-----+-----+

   Key value (the "hex code" used in opcodes):

   +-----+-----+-----+-----+
   | 0x1 | 0x2 | 0x3 | 0xC |
   +-----+-----+-----+-----+
   | 0x4 | 0x5 | 0x6 | 0xD |
   +-----+-----+-----+-----+
   | 0x7 | 0x8 | 0x9 | 0xE |
   +-----+-----+-----+-----+
   | 0xA | 0x0 | 0xB | 0xF |
   +-----+-----+-----+-----+

   * FX0A halts the CPU until a key is pressed,
     then stores its value in VX.
```

---

## 7. Fontset in Memory

Each of the 16 hex characters is 4 pixels wide and 5 pixels
tall, encoded as 5 bytes. Total = 80 bytes, usually loaded at
0x050.

```
   Memory address   Character   Bytes (binary)
   --------------   ---------   --------------------------------------
   0x050  "0"       0xF0 0x90 0x90 0x90 0xF0
                  +-+
                  | |           1111 0000
                  | |           1001 0000
                  | |           1001 0000
                  | |           1001 0000
                  +-+           1111 0000

   0x055  "1"       0x20 0x60 0x20 0x20 0x70
                    .
                    |
                    |
                    |
                    |

   0x05A  "2"       0xF0 0x10 0xF0 0x80 0xF0
                  +-+            +-+
                    |          +-+
                  +-+            |
                  +-+

   ... etc for 3..F ...

   * The "A" character is the FIRST one used in many games
     (0x0A -> addr 0x00A in fontset -> 0x050 + 0x0A*5 = 0x09A).
   * Address of a character in memory:
        addr = 0x050 + (char_value) * 5
```

---

## 8. Opcode Format

Every Chip8 opcode is 2 bytes. We read them big-endian and
split into 4 nibbles.

```
   Byte 1                Byte 2
   +----+----+----+----+ +----+----+----+----+
   | 15 | 14 | 13 | 12 | | 11 | 10 |  9 |  8 |  ... 4 bits each
   +----+----+----+----+ +----+----+----+----+
     ^     ^    ^    ^      ^     ^    ^    ^
     |     |    |    |      |     |    |    |
     |     |    |    +-- N  |     |    +-- NN low
     |     |    +----- Y    |     +------- X
     |     +---------- X
     +-------------- OP

   The four most common opcode shapes:

   +------+------+------+------+----------------------+
   |  6X  |  NN  |          | Set VX = NN           |
   |  7X  |  NN  |          | ADD VX, NN            |
   |  ANN |  NNN |          | Set I = NNN           |
   |  DXYN|      |          | Draw N-byte sprite    |
   +------+------+------+------+----------------------+
```

---

## 9. Fetch / Decode / Execute Cycle

This is the heart of the CPU. One iteration = one "tick".

```
        +-----------------------------------+
        |                                   |
        v                                   |
   +------------+                            |
   |   Fetch    |   opcode = (mem[PC] << 8)  |
   |            |             | mem[PC + 1]  |
   +-----+------+                            |
         |                                   |
         v                                   |
   +------------+                            |
   |  Decode    |   extract: OP, X, Y, N,    |
   |            |           NN, NNN          |
   +-----+------+                            |
         |                                   |
         v                                   |
   +------------+                            |
   |  Execute   |   run the matching         |
   |            |   instruction, mutating     |
   |            |   registers / memory        |
   +-----+------+                            |
         |                                   |
         |  PC += 2 (unless the op jumped) --+
         |
         v
   (repeat)
```

### 9b. Frame loop (the outer loop)

A "frame" runs many CPU cycles, then updates timers and
re-draws the display.

```
   +----------------+
   |  Load ROM      |
   |  Reset CPU     |
   +-------+--------+
           |
           v
   +----------------+     +-----------------+
   |  Run ~500-1000 |---> |  Update timers  |
   |  CPU cycles    |     |  DT--, ST--     |
   |  (one opcode)  |     |  (at 60 Hz)     |
   +-------+--------+     +-----------------+
           |
           v
   +----------------+
   |  Draw display  |  <-- paint pixels to screen
   +-------+--------+
           |
           v
   +----------------+
   |  Poll keys     |
   +-------+--------+
           |
           +-------> back to "Run CPU cycles"
```

---

## 10. Useful Constant Cheat-Sheet

```
   MEMORY_SIZE            4096
   PROGRAM_START          0x200
   FONTSET_START          0x050
   FONTSET_SIZE           80        (16 chars * 5 bytes)
   STACK_SIZE             16
   VIDEO_WIDTH            64
   VIDEO_HEIGHT           32
   VIDEO_PIXELS           2048      (64 * 32)
   REGISTER_COUNT         16
   KEY_COUNT              16
   TIMER_HZ               60
   CPU_HZ (target)        ~500
   OPCODE_SIZE            2 bytes
```

---

## 11. Endianness (gotcha for C)

Chip8 ROMs are big-endian. If you read two bytes sequentially
from a file into memory at index `i` and `i+1`, the high byte
comes first:

```
   File bytes:       [ hi ] [ lo ]  -> opcode stored at mem[PC], mem[PC+1]
                       ^      ^
                       |      +---  low 8 bits
                       +----------  high 8 bits

   opcode = (mem[PC] << 8) | mem[PC + 1];

   e.g.  bytes 0xA2 0xF0  ->  opcode 0xA2F0  ->  ANNN with NNN=0x2F0
```
