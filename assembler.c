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
Labels *populate_labels(Program *prog)
{
    static int address;
    if(prog == NULL)
    {
        address = 0;
        return NULL;
    }
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
        tmp->address = address;
        tmp->next = ret;
        ret = tmp;
    }
    if(prog->type == INSTRUCTION)
        address += opcode_get_expanded_size(prog->instr->op);
    else if(prog->type == DATA)
        address++;
    return ret;
}
int check_arglist(OpCode op, Arguments *args)
{
    if(count_args(args) != opcode_get_nargs(op))
    {
        printf("error: %s needs %d arguments !\n", opcode_get_string(op), opcode_get_nargs(op));
        exit(1);
    }
    switch(opcode_get_arglist(op))
    {
    case ARGS_NUM:
        if(args->type != NUM)
        {
            printf("%s needs a value as argument !\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_REG_REG:
        if(args->type != REG || args->previous->type != REG)
        {
            printf("%s needs registers as first and second argument !\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_REG_NUM:
        if(args->type != NUM || args->previous->type != REG)
        {
            printf("%s needs register as first argument and value as second argument !\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_REG:
        if(args->type != REG)
        {
            printf("%s needs a registers as argument !\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_POS_POS_REG:
        if(args->type != REG || args->previous->type != POS || args->previous->previous->type != POS)
        {
            printf("%s needs positions as first and second argument and register as third argument !\n", opcode_get_string(op));
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
            printf("%s needs position as first argument and register as second argument !\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_REG_POS:
        if(args->type != POS || args->previous->type != REG)
        {
            printf("%s needs register as first argument and position as second argument !\n", opcode_get_string(op));
            exit(1);
        }
        break;
        //case ARGS_EMPTY: //Inutile, à voir
        //if(args->type != REG || args->previous->type != REG)
        //{
        //  printf("%s needs registers as first and second argument !\n", opcode_get_string(op));
        //exit(1);
        // }
        //break;
    case ARGS_LAB:
        if(args->type != IDENT) // A VOIR : IDENT <=> LAB ?
        {
            printf("%s needs label as first argument !\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_REG_LAB:
        if(args->type != IDENT || args->previous->type != REG)
        {
            printf("%s needs register as first argument and label as second argument !\n", opcode_get_string(op));
            exit(1);
        }
        break;
    case ARGS_LAB_REG:
        if(args->type != REG || args->previous->type != IDENT) // A VOIR : IDENT <=> LAB ?
        {
            printf("%s needs label as first argument and register as second argument !\n", opcode_get_string(op));
            exit(1);
        }
        break;
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
Program *make16(int val)
{
    //
    //
    //
}
Program *make32(int val)
{
    //
    //
    //
}
Program *expand_macros(Program *prog, Labels *labels)
{
    if(prog == NULL)
        return prog;
    prog->previous = expand_macros(prog->previous, labels);
    if(prog->type != INSTRUCTION)
        return prog;
    int val, address;
    Cond cond;
    char *ident;
    switch(prog->instr->op)
    {
    case MAKE16:
        //
        //
        //
    case MAKE32:
        val = prog->instr->args->num;
        cond = prog->instr->cond;
        prog = erase_line(prog);
        prog = insert_prog(prog, make32(val));
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
        prog = insert_prog(prog, make32(address - count_lines(prog) - opcode_get_expanded_size(JUMP)));
        prog = insert_line(prog, "JMP NC $0");
        break;
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
        switch(prog->instr->op)
        {
        case MAKE:
				word = prog->instr->args->num & (1<<8 - 1); /* Facultatif ? Sélectionne les 8 premiers bits seulement */
				break;

        case NOP:
				word = 0;
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
            word = prog->instr->args->num;
				word <<= 4;
				word += prog->instr->args->previous->num;
				break;

        case ADDI:
            word = prog->instr->args->previous->num;
            word <<= 4;
            if(prog->instr->args->num < 0)
                word |= 0x8;
            word += (prog->instr->args->num < 0 ? -prog->instr->args->num : prog->instr->args->num) & 0x7;
            break;

        case JMP:
				word = prog->instr->args->previous->num;
            word <<= 4;
            break;

        case GETSR:
				word = prog->instr->args->num;
				break;

        case CALL:
				word = (prog->instr->args->num) << 4;
				break;

        case MOVE16: //
        case MOVE32: //

        case SETSR:
            word = prog->instr->args->num;
				break;

        case PUSH:
            word = pos_get_code(prog->instr->args->previous->pos);
            word <<= 4;
            word += prog->instr->args->num;
            break;

        case POP:
            //
            //
            //
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
    prog = expand_macros(prog, labels);
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
