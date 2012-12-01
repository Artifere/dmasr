#include "instruction_set.h"
int arglist_get_nargs(ArgList l)
{
    switch(l)
    {
    case ARGS_NUM:
        return 1;
    case ARGS_REG_REG:
        return 2;
    case ARGS_REG_NUM:
        return 2;
    case ARGS_REG:
        return 1;
    case ARGS_POS_POS_REG:
        return 3;
    case ARGS_POS_POS_REG_REG:
        return 4;
    case ARGS_POS_REG:
        return 2;
    case ARGS_REG_POS:
        return 2;
    case ARGS_EMPTY:
        return 0;
    case ARGS_REG_LAB:
        return 2;
    case ARGS_LAB_REG:
        return 2;
    case ARGS_LAB:
        return 1;
    }
}
int opcode_get_expanded_size(OpCode code)
{
    switch(code)
    {
    case MAKE:
        return 1;
    case NOP:
        return 1;
    case ADD:
        return 1;
    case SUB:
        return 1;
    case AND:
        return 1;
    case OR:
        return 1;
    case NOT:
        return 1;
    case XOR:
        return 1;
    case ADDI:
        return 1;
    case SHL:
        return 1;
    case SHR:
        return 1;
    case LDR:
        return 1;
    case STR:
        return 1;
    case JMP:
        return 1;
    case ADDP16:
        return 1;
    case SUBV16:
        return 1;
    case ADDV8:
        return 1;
    case ADDV16:
        return 1;
    case MULV16:
        return 1;
    case ADDR16:
        return 1;
    case SUBV8:
        return 1;
    case MOVE16:
        return 1;
    case MOVE32:
        return 1;
    case MOVE64:
        return 1;
    case GETSR:
        return 1;
    case SETSR:
        return 1;
    case COMP_GT:
        return 1;
    case COMP_LE:
        return 1;
    case PUSH:
        return 1;
    case POP:
        return 1;
    case RET:
        return 1;
    case CALL:
        return 1;

    case MAKE16: //////
    case MAKE32: //////

    case JUMP:   //////
        break;
    }
}
ArgList opcode_get_arglist(OpCode code)
{
    switch(code)
    {
    case MAKE:
        return ARGS_NUM;
    case NOP:
        return ARGS_EMPTY;
    case ADD:
        return ARGS_REG_REG;
    case SUB:
        return ARGS_REG_REG;
    case AND:
        return ARGS_REG_REG;
    case OR:
        return ARGS_REG_REG;
    case NOT:
        return ARGS_REG_REG;
    case XOR:
        return ARGS_REG_REG;
    case ADDI:
        return ARGS_REG_NUM;
    case SHL:
        return ARGS_REG_REG;
    case SHR:
        return ARGS_REG_REG;
    case LDR:
        return ARGS_REG_LAB; // ????
    case STR:
        return ARGS_LAB_REG;
    case JMP:
        return ARGS_REG;
    case ADDP16:
        return ARGS_REG_REG;
    case SUBV16:
        return ARGS_REG_REG;
    case ADDV8:
        return ARGS_REG_REG;
    case ADDV16:
        return ARGS_REG_REG;
    case MULV16:
        return ARGS_REG_REG;
    case ADDR16:
        return ARGS_REG_REG;
    case SUBV8:
        return ARGS_REG_REG;
    case MOVE16:
        return ARGS_POS_POS_REG;
    case MOVE32:
        return ARGS_POS_POS_REG_REG;
    case MOVE64:
        return ARGS_REG_REG;
    case GETSR:
        return ARGS_REG;
    case SETSR:
        return ARGS_REG;
    case COMP_GT:
        return ARGS_REG_REG;
    case COMP_LE:
        return ARGS_REG_REG;
    case PUSH:
        return ARGS_POS_REG;
    case POP:
        return ARGS_REG_POS;
    case RET:
        return ARGS_EMPTY;
    case CALL:
        return ARGS_LAB;

    case MAKE16://////
    case MAKE32://////

    case JUMP://////
        break;
    }
}
int opcode_get_nargs(OpCode code)
{
    return arglist_get_nargs(opcode_get_arglist(code));
}
int opcode_get_code(OpCode code)
{
    switch(code)
    {
    case MAKE:
        return 0x00;
    case NOP:
        return 0x01;
    case ADD:
        return 0x02;
    case SUB:
        return 0x03;
    case AND:
        return 0x04;
    case OR:
        return 0x05;
    case NOT:
        return 0x06;
    case XOR:
        return 0x07;
    case ADDI:
        return 0x08;
    case SHL:
        return 0x09;
    case SHR:
        return 0x0a;
    case LDR:
        return 0x0b;
    case STR:
        return 0x0c;
    case JMP:
        return 0x0d;
    case ADDP16:
        return 0x0e;
    case SUBV16:
        return 0x0f;
    case ADDV8:
        return 0x10;
    case ADDV16:
        return 0x11;
    case MULV16:
        return 0x12;
    case ADDR16:
        return 0x13;
    case SUBV8:
        return 0x14;
    case MOVE16:
        return 0x15;
    case MOVE32:
        return 0x16;
    case MOVE64:
        return 0x17;
    case GETSR:
        return 0x18;
    case SETSR:
        return 0x19;
    case COMP_GT:
        return 0x1a;
    case COMP_LE:
        return 0x1b;
    case PUSH:
        return 0x1c;
    case POP:
        return 0x1d;
    case RET:
        return 0x1e;
    case CALL:
        return 0x1f;
    }
    return -1;
}
char *opcode_get_string(OpCode code)
{
    switch(code)
    {
    case MAKE:
        return "MAKE";
    case NOP:
        return "NOP";
    case ADD:
        return "ADD";
    case SUB:
        return "SUB";
    case AND:
        return "AND";
    case OR:
        return "OR";
    case NOT:
        return "NOT";
    case XOR:
        return "XOR";
    case ADDI:
        return "ADDI";
    case SHL:
        return "SHL";
    case SHR:
        return "SHR";
    case LDR:
        return "LDR";
    case STR:
        return "STR";
    case JMP:
        return "JMP";
    case ADDP16:
        return "ADDP16";
    case SUBV16:
        return "SUBV16";
    case ADDV8:
        return "ADDV8";
    case ADDV16:
        return "ADDV16";
    case MULV16:
        return "MULV16";
    case ADDR16:
        return "ADDR16";
    case SUBV8:
        return "SUBV8";
    case MOVE16:
        return "MOVE16";
    case MOVE32:
        return "MOVE32";
    case MOVE64:
        return "MOVE64";
    case GETSR:
        return "GETSR";
    case SETSR:
        return "SETSR";
    case COMP_GT:
        return "COMP.GT";
    case COMP_LE:
        return "COMP.LE";
    case PUSH:
        return "PUSH";
    case POP:
        return "POP";
    case RET:
        return "RET";
    case CALL:
        return "CALL";

    case MAKE16:
        return "MAKE16";
    case MAKE32:
        return "MAKE32";

    case JUMP:
        return "JUMP";
    }
    return "";
}
int cond_get_code(Cond cond)
{
    switch(cond)
    {
    case COND_NZ:
        return 0x0;
    case COND_Z:
        return 0x1;
    case COND_GE:
        return 0x2;
    case COND_LE:
        return 0x3;
    case COND_L:
        return 0x4;
    case COND_C:
        return 0x5;
    case COND_V:
        return 0x6;
    case COND_NC:
        return 0x7;
    }
    return -1;
}
char *cond_get_string(Cond cond)
{
    switch(cond)
    {
    case COND_NZ:
        return "NZ";
    case COND_Z:
        return "Z";
    case COND_GE:
        return "GE";
    case COND_LE:
        return "LE";
    case COND_L:
        return "L";
    case COND_C:
        return "C";
    case COND_V:
        return "V";
    case COND_NC:
        return "NC";
    }
    return "";
}
int pos_get_code(Pos pos)
{
    switch(pos)
    {
    case POS_H:
        return 1;
    case POS_L:
        return 0;
    case POS_HH:
        return 3;
    case POS_HL:
        return 2;
    case POS_LH:
        return 1;
    case POS_LL:
        return 0;
    }
    return -1;
}
char *pos_get_string(Pos pos)
{
    switch(pos)
    {
    case POS_H:
        return "H";
    case POS_L:
        return "L";
    case POS_HH:
        return "HH";
    case POS_HL:
        return "HL";
    case POS_LH:
        return "LH";
    case POS_LL:
        return "LL";
    }
    return "";
}
