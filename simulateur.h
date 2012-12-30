#define FBSA 0x00200000
#include <gtk/gtk.h>
typedef GHashTable Mem;
typedef struct State {
  Mem *mem;
  int reg[16][2];
  int PC;
  int SP;
  int RA;
  char SR;
} State;
Mem *write_mem(Mem *mem, int address, int value);
int read_mem(Mem *mem, int address);
Mem *free_mem(Mem *mem);
void execute(State *s);
