#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "parser.h"

int get_address(Labels *labels, char *label)
{
    if(labels == NULL)
        return -1;
    if(strcmp(labels->label, label) == 0)
        return labels->address;
    return get_address(labels->next, label);
}
int get_value(Data *data, char *label)
{
    if(data == NULL)
        return -1;
    if(strcmp(data->label, label) == 0)
        return data->num;
    return get_value(data->next, label);
}
int count_out_lines(Program *prog)
{
    if(prog == NULL)
        return 0;
    if(prog->type == LABEL)
        return count_out_lines(prog->previous);
    if(prog->type == INSTRUCTION)
        return count_out_lines(prog->previous) + opcode_get_expanded_size(prog->instr->op);
    return count_out_lines(prog->previous) + 1;
}
Labels *populate_labels(Program *prog)
{
    if(prog == NULL)
        return NULL;
    Labels *ret = populate_labels(prog->previous);
    if(prog->type == LABEL)
    {
        if(get_address(ret, prog->label) != -1)
        {
            printf("Label defined twice: %s\n", prog->label);
            exit(1);
        }
        Labels *tmp = malloc(sizeof(Labels));
        tmp->label = malloc((strlen(prog->label) + 1) * sizeof(char));
        strcpy(tmp->label, prog->label);
        tmp->address = count_out_lines(prog);
        tmp->next = ret;
        ret = tmp;
    }
    return ret;
}

Data *populate_data(Program *prog)
{
    if(prog == NULL)
        return NULL;
    Data *ret = populate_data(prog->previous);
    if(prog->type == DATA &&prog->previous != NULL && prog->previous->type == LABEL)
    {
        if(get_value(ret, prog->previous->label) != -1)
        {
            printf("Label defined twice: %s\n", prog->label);
            exit(1);
        }
        Data *tmp = malloc(sizeof(Data));
        tmp->label = malloc((strlen(prog->previous->label) + 1) * sizeof(char));
        strcpy(tmp->label, prog->previous->label);
        tmp->num = prog->num;
        tmp->next = ret;
        ret = tmp;
    }
    return ret;
}
int check_arglist(OpCode op, Arguments *args)
{
    if(count_args(args) != opcode_get_nargs(op))
    {
        printf("error: %s needs %d arguments!\n", opcode_get_string(op), opcode_get_nargs(op));
        exit(1);
    }
    switch(opcode_get_arglist(op))
    {
    case ARGS_NUM:
        if(args->type != NUM)
        {
            printf("%s needs a value as argument!\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_REG_REG:
        if(args->type != REG || args->previous->type != REG)
        {
            printf("%s needs registers as first and second argument!\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_REG_NUM:
        if(args->type != NUM || args->previous->type != REG)
        {
            printf("%s needs a value as first argument!\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_REG:
        if(args->type != REG)
        {
            printf("%s needs a registers as argument!\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_POS_POS_REG:
        if(args->type != REG || args->previous->type != POS || args->previous->previous->type != POS)
        {
            printf("%s needs positions as first and second argument and register as third argument!\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_POS_POS_REG_REG:
        if(args->type != REG || args->previous->type != REG || args->previous->previous->type != POS || args->previous->previous->previous->type != POS)
        {
            printf("%s needs positions as first and second argument and registers as third and fourth argument!\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_POS_REG:
        if(args->type != REG || args->previous->type != POS)
        {
            printf("%s needs position as first argument and register as second argument!\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_REG_POS:
        if(args->type != POS || args->previous->type != REG)
        {
            printf("%s needs register as first argument and position as second argument!\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_EMPTY:
        break;

    case ARGS_LAB:
        if(args->type != IDENT)
        {
            printf("%s needs label as first argument!\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_REG_LAB:
        if(args->type != IDENT || args->previous->type != REG)
        {
            printf("%s needs register as first argument and label as second argument!\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_LAB_REG:
        if(args->type != REG || args->previous->type != IDENT)
        {
            printf("%s needs label as first argument and register as second argument!\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_NUM_NUM:
        if(args->type != NUM || args->previous->type != NUM)
        {
            printf("%s needs a value as first and second argument!\n", opcode_get_string(op));
            exit(1);
        }
    }
}
void check_program_arglist(Program *prog)
{
    if(prog == NULL)
        return;
    check_program_arglist(prog->previous);
    if(prog->type == INSTRUCTION)
        check_arglist(prog->instr->op, prog->instr->args);
}
Program *insert_line(Program *prog, char *line)
{
    Program *prog_tmp = parse_line(line);
    prog_tmp->previous = prog;
    return prog_tmp;
}
Program *erase_line(Program *prog)
{
    Program *prog_tmp = prog->previous;
    prog->previous = NULL;
    free_program(prog);
    return prog_tmp;
}
Program *insert_prog(Program *prog, Program *prog2)
{
    Program *prog_tmp = prog2;
    while(prog_tmp->previous != NULL)
        prog_tmp = prog_tmp->previous;
    prog_tmp->previous = prog;
    return prog2;
}



Arguments *add_arg(int type, int num, char *ident, Pos pos, Arguments *cur)
{
    Arguments *ret = malloc(sizeof(*ret));
    ret->num = num;
    ret->ident = ident;
    ret->pos = pos;
    ret->type = type;

    ret->previous = cur;

    return ret;
}


Program *add_prog(OpCode op, Cond cond, Arguments *args, Program *cur)
{
    Instruction *i = malloc(sizeof(*i));
    i->op = op;
    i->cond = cond;
    i->args = args;

    Program *ret = malloc(sizeof(*ret));
    ret->type = INSTRUCTION;
    ret->num = 0;
    ret->label = NULL;
    ret->instr = i;
    ret->previous = cur;


    return ret;
}

Program *make16(int val, Cond macro_cond)
{
    int lower = (val & 0xff);
    int upper = (val >> 8) & 0xff;
    Program *prog_head = NULL;
    Arguments *arg_head = NULL;

    //1.PUSH L $2
    arg_head = add_arg(POS, 0, NULL, POS_L, NULL);
    arg_head = add_arg(REG, 2, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, NULL);
    //PUSH L $1
    arg_head = add_arg(POS, 0, NULL, POS_L, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, prog_head);
    //PUSH H $1
    arg_head = add_arg(POS, 0, NULL, POS_H, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, prog_head);
    //GETSR $2
    arg_head = add_arg(REG, 2, NULL, 0, NULL);
    prog_head = add_prog(GETSR, COND_NC, arg_head, prog_head);
    //5.SUB $1 $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //ADDI $1 7
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, 7, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //ADDI $1 7
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, 7, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //ADDI $1 1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, 1, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //JMP $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    prog_head = add_prog(JMP, COND_NC, arg_head, prog_head);


    //10.POP L $0
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_L, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);
    //PUSH H $0
    arg_head = add_arg(POS, 0, NULL, POS_H, NULL);
    arg_head = add_arg(REG, 0, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, prog_head);
    //SUB $1 $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //ADDI $1 3
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, 3, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //ADDI $1 5
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, 5, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //15.SHR $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHR, COND_NC, arg_head, prog_head);
    //SHR $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHR, COND_NC, arg_head, prog_head);
    //SHL $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHL, COND_NC, arg_head, prog_head);
    //MAKE *les 8 bits de poids fort*
    arg_head = add_arg(NUM, upper, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //SHL $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHL, COND_NC, arg_head, prog_head);
    //20.MAKE *les 8 bits de poids faible*
    arg_head = add_arg(NUM, lower, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //POP H $0
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_H, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);
    //ADDI $1 1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, 1, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //JMP $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    prog_head = add_prog(JMP, COND_NC, arg_head, prog_head);


    //SUB $1 $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //25.PUSH L $0
    arg_head = add_arg(POS, 0, NULL, POS_L, NULL);
    arg_head = add_arg(REG, 0, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, prog_head);
    //SUB $0 $0
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 0, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //MAKE 20
    arg_head = add_arg(NUM, 20, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //SUB $1 $0
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(REG, 0, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //SETSR  $2
    arg_head = add_arg(REG, 2, NULL, 0, NULL);
    prog_head = add_prog(SETSR, COND_NC, arg_head, prog_head);
    //J30.MP *conditions de la macro *$1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    prog_head = add_prog(JMP, macro_cond, arg_head, prog_head);
    //POP L $0
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_L, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);


    //POP H $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_H, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);
    //POP L $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_L, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);
    //34.POP L $2
    arg_head = add_arg(REG, 2, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_L, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);

    return prog_head;
}

Program *make32(int val, Cond macro_cond)
{
    int lower_l = (val & 0xff);
    int lower_h = (val >> 8) & 0xff;
    int upper_l = (val >> 16) & 0xff;
    int upper_h = (val >> 24) & 0xff;
    Program *prog_head = NULL;
    Arguments *arg_head = NULL;


    //1.PUSH L $2
    arg_head = add_arg(POS, 0, NULL, POS_L, NULL);
    arg_head = add_arg(REG, 2, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, NULL);
    //PUSH L $1
    arg_head = add_arg(POS, 0, NULL, POS_L, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, prog_head);
    //PUSH H $1
    arg_head = add_arg(POS, 0, NULL, POS_H, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, prog_head);
    //GETSR $2
    arg_head = add_arg(REG, 2, NULL, 0, NULL);
    prog_head = add_prog(GETSR, COND_NC, arg_head, prog_head);
    //5.SUB $0 $0
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 0, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //MAKE 16
    arg_head = add_arg(NUM, 16, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //JMP $0
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    prog_head = add_prog(JMP, COND_NC, arg_head, prog_head);



    //PUSH H $0
    arg_head = add_arg(POS, 0, NULL, POS_H, NULL);
    arg_head = add_arg(REG, 0, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, prog_head);
    //SUB $0 $0
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 0, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //10.SUB $1 $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //ADDI $1 3
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, 3, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //ADDI $1 5
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, 5, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //MAKE *les 8 bits forts de poids fort*
    arg_head = add_arg(NUM, upper_h, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //SHL $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHL, COND_NC, arg_head, prog_head);
    //15.MAKE *les 8 bits faibles de poids fort*
    arg_head = add_arg(NUM, upper_l, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //SHL $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHL, COND_NC, arg_head, prog_head);
    //MAKE *les 8 bits forts de poids faible*
    arg_head = add_arg(NUM, lower_h, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //SHL $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHL, COND_NC, arg_head, prog_head);
    //MAKE *les 8 bits faibles de poids faible*
    arg_head = add_arg(NUM, lower_l, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //20.POP H $0
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_H, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);
    //ADDI $1 -2
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, -2, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //JMP $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    prog_head = add_prog(JMP, COND_NC, arg_head, prog_head);


    //SUB $1 $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //MAKE 19
    arg_head = add_arg(NUM, 19, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //25.SUB $1 $0
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(REG, 0, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //SETSR  $2
    arg_head = add_arg(REG, 2, NULL, 0, NULL);
    prog_head = add_prog(SETSR, COND_NC, arg_head, prog_head);
    //JMP *conditions de la macro *$1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    prog_head = add_prog(JMP, macro_cond, arg_head, prog_head);



    //POP H $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_H, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);
    //POP L $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_L, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);
    //30.POP L $2
    arg_head = add_arg(REG, 2, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_L, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);

    return prog_head;
}


Program *make64(int val_h, int val_l, Cond macro_cond)
{
    int lll = (val_l & 0xff);
    int llh = (val_l >> 8) & 0xff;
    int lhl = (val_l >> 16) & 0xff;
    int lhh = (val_l >> 24) & 0xff;
    int hll = (val_h & 0xff);
    int hlh = (val_h >> 8) & 0xff;
    int hhl = (val_h >> 16) & 0xff;
    int hhh = (val_h >> 24) & 0xff;
    Program *prog_head = NULL;
    Arguments *arg_head = NULL;


    //1.PUSH L $2
    arg_head = add_arg(POS, 0, NULL, POS_L, NULL);
    arg_head = add_arg(REG, 2, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, NULL);
    //PUSH L $1
    arg_head = add_arg(POS, 0, NULL, POS_L, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, prog_head);
    //PUSH H $1
    arg_head = add_arg(POS, 0, NULL, POS_H, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, prog_head);
    //GETSR $2
    arg_head = add_arg(REG, 2, NULL, 0, NULL);
    prog_head = add_prog(GETSR, COND_NC, arg_head, prog_head);
    //5.MAKE 23
    arg_head = add_arg(NUM, 23, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //JMP $0
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    prog_head = add_prog(JMP, COND_NC, arg_head, prog_head);



    //SUB $0 $0
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 0, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //SUB $1 $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //ADDI $1 3
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, 3, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //10.ADDI $1 5
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, 5, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //MAKE *les 8 bits forts de poids fort, moitié haute*
    arg_head = add_arg(NUM, hhh, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //SHL $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHL, COND_NC, arg_head, prog_head);
    //MAKE *les 8 bits faibles de poids fort, moitié haute*
    arg_head = add_arg(NUM, hhl, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //SHL $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHL, COND_NC, arg_head, prog_head);
    //15.MAKE *les 8 bits forts de poids faible, moitié haute*
    arg_head = add_arg(NUM, hlh, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //SHL $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHL, COND_NC, arg_head, prog_head);
    //MAKE *les 8 bits faibles de poids faible, moitié haute*
    arg_head = add_arg(NUM, hll, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //PUSH L $0
    arg_head = add_arg(POS, 0, NULL, POS_L, NULL);
    arg_head = add_arg(REG, 0, NULL, 0, arg_head);
    prog_head = add_prog(PUSH, COND_NC, arg_head, prog_head);
    //MAKE *les 8 bits forts de poids fort, moitié basse*
    arg_head = add_arg(NUM, lhh, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //20.SHL $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHL, COND_NC, arg_head, prog_head);
    //MAKE *les 8 bits faibles de poids fort, moitié basse*
    arg_head = add_arg(NUM, lhl, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //SHL $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHL, COND_NC, arg_head, prog_head);
    //MAKE *les 8 bits forts de poids faible, moitié basse*
    arg_head = add_arg(NUM, llh, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //SHL $0 $1
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SHL, COND_NC, arg_head, prog_head);
    //25.MAKE *les 8 bits faibles de poids faible, moitié basse*
    arg_head = add_arg(NUM, lll, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //POP $0 H
    arg_head = add_arg(REG, 0, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_H, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);
    //ADDI $1 -2
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(NUM, -2, NULL, 0, arg_head);
    prog_head = add_prog(ADDI, COND_NC, arg_head, prog_head);
    //JMP $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    prog_head = add_prog(JMP, COND_NC, arg_head, prog_head);


    //SUB $1 $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(REG, 1, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //30.MAKE 26
    arg_head = add_arg(NUM, 26, NULL, 0, NULL);
    prog_head = add_prog(MAKE, COND_NC, arg_head, prog_head);
    //SUB $1 $0
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(REG, 0, NULL, 0, arg_head);
    prog_head = add_prog(SUB, COND_NC, arg_head, prog_head);
    //SETSR  $2
    arg_head = add_arg(REG, 2, NULL, 0, NULL);
    prog_head = add_prog(SETSR, COND_NC, arg_head, prog_head);
    //JMP *conditions de la macro *$1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    prog_head = add_prog(JMP, macro_cond, arg_head, prog_head);



    //POP H $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_H, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);
    //35.POP L $1
    arg_head = add_arg(REG, 1, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_L, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);
    //36.POP L $2
    arg_head = add_arg(REG, 2, NULL, 0, NULL);
    arg_head = add_arg(POS, 0, NULL, POS_L, arg_head);
    prog_head = add_prog(POP, COND_NC, arg_head, prog_head);

    return prog_head;
}


Program *expand_macros(Program *prog, Labels *labels, Data *data)
{
    if(prog == NULL)
        return prog;
    prog->previous = expand_macros(prog->previous, labels, data);
    if(prog->type != INSTRUCTION)
        return prog;
    int val, address;
    int val_h, val_l;
    Cond cond;
    char *ident;
    Arguments* arg_head;
    switch(prog->instr->op)
    {
    case MAKE16:
        val = prog->instr->args->num;
        cond = prog->instr->cond;
        prog = erase_line(prog);
        prog = insert_prog(prog, make16(val, cond));
        break;

    case MAKE32:
        val = prog->instr->args->num;
        cond = prog->instr->cond;
        prog = erase_line(prog);
        prog = insert_prog(prog, make32(val, cond));
        break;

    case MAKE64:
        val_h = prog->instr->args->previous->num;
        val_l = prog->instr->args->num;
        cond = prog->instr->cond;
        prog = erase_line(prog);
        prog = insert_prog(prog, make64(val_h, val_l, cond));
        break;

    case JUMP:
        address = get_address(labels, prog->instr->args->ident);
        cond = prog->instr->cond;
        if(address == -1)
        {
            printf("Unknown label: %s !\n", prog->instr->args->ident);
            exit(1);
        }
        prog = erase_line(prog);
        prog = insert_line(prog, "PUSH NC H,$0");
        prog = insert_line(prog, "PUSH NC L,$1");
        prog = insert_line(prog, "GETSR NC $1");
        prog = insert_line(prog, "SUB NC $0,$0");
        prog = insert_line(prog, "ADDI NC $0,1");
        prog = insert_line(prog, "SETSR NC $1");
        prog = insert_prog(prog, make32(address - count_out_lines(prog) - opcode_get_expanded_size(JUMP) + 1, cond));
        //si cond n'est pas verifiee, on a 1 dans $0 et le jmp ne fait rien
        prog = insert_line (prog, "POP NC $1,L");
        prog = insert_line (prog, "POP NC $0,H");
        prog = insert_line(prog, "JMP NC $0");
        break;

    case CALLF:
        address = get_address(labels, prog->instr->args->ident);
        cond = prog->instr->cond;
        if(address == -1)
        {
            printf("Unknown label: %s !\n", prog->instr->args->ident);
            exit(1);
        }
        prog = erase_line(prog);
        prog = insert_prog (prog, make32(address, COND_NC));
        prog = insert_line (prog, "CALL NC $0");
        break;

    case LOAD:
        val = get_value(data, prog->instr->args->ident);
        cond = prog->instr->cond;
        prog = erase_line(prog);
        prog = insert_prog(prog, make16(val, COND_NC));
        arg_head = add_arg(REG, prog->instr->args->previous->num, NULL, 0, NULL);
        arg_head = add_arg(REG, 0, NULL, 0, arg_head);
        prog = add_prog(LDR, cond, arg_head, prog);
        break;

    case STORE:
        val = get_value(data, prog->instr->args->previous->ident);
        cond = prog->instr->cond;
        prog = erase_line(prog);
        prog = insert_prog(prog, make16(val, COND_NC));
        arg_head = add_arg(REG, prog->instr->args->num, NULL, 0, NULL);
        arg_head = add_arg(REG, 0, NULL, 0, arg_head);
        prog = add_prog(STR, cond, arg_head, prog);
    }


    return prog;
}
void write_prog(Program *prog, FILE *file)
{
    if(prog == NULL)
        return;
    write_prog(prog->previous, file);
    char word;
    switch(prog->type)
    {
    case INSTRUCTION:
        word = opcode_get_code(prog->instr->op);
        word <<= 3;
        word += cond_get_code(prog->instr->cond);
        fputc(word, file);
        word = 0;
        switch(prog->instr->op)
        {
        case MAKE:
            word = prog->instr->args->num;
            break;

        case NOP:
            break;

        case ADD:
        case SUB:
        case AND:
        case OR:
        case NOT:
        case XOR:
        case SHL:
        case SHR:
        case LDR:
        case STR:
        case ADDP16:
        case SUBV16:
        case ADDV8:
        case ADDV16:
        case MULV16:
        case ADDR16:
        case SUBV8:
        case MOVE64:
        case COMP_GT:
        case COMP_LE:
            word = prog->instr->args->previous->num;
            word <<= 4;
            word += prog->instr->args->num;
            break;
        case ADDI:
            word = prog->instr->args->previous->num;
            word <<= 4;
            if(prog->instr->args->num < 0)
                word |= 0x8;
            //Correction ADDI
            word += ((prog->instr->args->num < 0 ? (8 +prog->instr->args->num) : prog->instr->args->num) & 0x7);
            break;

        case JMP:
            word = prog->instr->args->num;
            word <<= 4;
            break;

        case GETSR:
            word = prog->instr->args->num;
            word <<= 4;
            break;

        case CALL:
            word = prog->instr->args->num << 4;
            break;

        case MOVE32:
            word = pos_get_code(prog->instr->args->previous->previous->previous->pos);
            word <<= 1;
            word += pos_get_code(prog->instr->args->previous->previous->pos);
            word <<= 2;
            word += prog->instr->args->previous->num;
            word <<= 4;
            word += prog->instr->args->num;
            break;

        case MOVE16:
            word = pos_get_code(prog->instr->args->previous->previous->pos);
            word <<= 2;
            word += pos_get_code(prog->instr->args->previous->pos);
            word <<= 4;
            word += prog->instr->args->num;
            break;

        case SETSR:
            word = prog->instr->args->num;
            break;

        case PUSH:
            word = pos_get_code(prog->instr->args->previous->pos);
            word <<= 4;
            word += prog->instr->args->num;
            break;

        case POP:
            word = prog->instr->args->previous->num;
            word <<= 4;
            word += pos_get_code(prog->instr->args->pos);
            break;

        case RET:
            word = 0;
            break;
        }
        fputc(word, file);
        break;
    case DATA:
        fputc((char)((prog->num >> 8) & 0xff), file);
        fputc((char)(prog->num & 0xff), file);
        break;
    case LABEL:
        break;
    }
}
int main(int argc, char **argv)
{
    int i;
    char *infile_name = NULL;
    char *outfile_name = NULL;
    for(i = 1; i < argc; i++)
        if(strcmp(argv[i], "-o") == 0)
        {
            i++;
            outfile_name = argv[i];
        }
        else
            infile_name = argv[i];
    if(infile_name == NULL)
    {
        printf("Please provide an input file name !\n");
        return 1;
    }
    if(outfile_name == NULL)
        outfile_name = "a.out";
    FILE *infile = fopen(infile_name, "r");
    if(infile == NULL)
    {
        printf("Error opening file: %s\n", infile_name);
        exit(1);
    }
    Program *prog = parse_program(infile);
    fclose(infile);
    check_program_arglist(prog);
    Labels *labels = populate_labels(prog);
    Data *data = populate_data(prog);
    prog = expand_macros(prog, labels, data);
    //Pour ne plus perdre 1h à chercher où est l'erreur quand on envoie pas les arguments dans le bon ordre...
    check_program_arglist(prog);
    FILE *outfile = fopen(outfile_name, "w");
    if(outfile == NULL)
    {
        printf("Error opening file: %s\n", outfile_name);
        exit(1);
    }
    write_prog(prog, outfile);
    free_program(prog);
    fclose(outfile);
    return 0;
}
