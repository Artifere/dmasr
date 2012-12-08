typedef struct Mem {
  int address;
  int value;
  struct Mem *next;
} Mem;
typedef struct State {
  struct Mem *mem;
  int reg[16][2];
  int PC;
  int SP;
  int RA;
  char SR;
} State;
