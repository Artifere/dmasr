#include <stdio.h>
#include "instruction_set.h"
typedef struct Arguments {
  enum {NUM, REG, IDENT, POS} type;
  int num;
  char *ident;
  Pos pos;
  struct Arguments *previous;
} Arguments;

typedef struct Instruction {
  OpCode op;
  Cond cond;
  Arguments *args;
} Instruction;

typedef struct Program {
  enum {INSTRUCTION, DATA, LABEL} type;
  Instruction *instr;
  int num;
  char *label;
  struct Program *previous;
} Program;

Program *parse_line(char *line);
Program *parse_program(FILE *file);
void free_program(Program *prog);
int count_lines(Program *prog);
int count_args(Arguments *args);
