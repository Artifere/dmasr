typedef enum {
  MAKE, NOP, ADD, SUB, AND, OR, NOT, XOR, ADDI, SHL, SHR, LDR, STR, JMP, ADDP16, SUBV16, ADDV8, ADDV16, MULV16, ADDR16, SUBV8, MOVE16, MOVE32, MOVE64, GETSR, SETSR, COMP_GT, COMP_LE, PUSH, POP, RET, CALL,
  MAKE16, MAKE32, MAKE64, JUMP, CALLF, LOAD, STORE
} OpCode;
#define MAX_OP_CODE STORE
typedef enum {
  COND_NZ, COND_Z, COND_GE, COND_LE, COND_L, COND_C, COND_V, COND_NC
} Cond;
#define MAX_COND COND_NC
typedef enum {
  POS_H, POS_L, POS_HH, POS_HL, POS_LH, POS_LL
} Pos;
#define MAX_POS POS_LL
typedef enum {
  ARGS_NUM, ARGS_REG_REG, ARGS_REG_NUM, ARGS_REG, ARGS_POS_POS_REG, ARGS_POS_POS_REG_REG, ARGS_POS_REG, ARGS_REG_POS, ARGS_EMPTY,
  ARGS_LAB, ARGS_REG_LAB, ARGS_LAB_REG, ARGS_NUM_NUM
} ArgList;
int opcode_get_nargs(OpCode code);
int opcode_get_code(OpCode code);
char *opcode_get_string(OpCode code);
int cond_get_code(Cond cond);
char *cond_get_string(Cond cond);
int pos_get_code(Pos pos);
char *pos_get_string(Pos pos);
