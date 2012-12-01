#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "simulateur.h"
Mem *write_mem(Mem *mem, int address, int value)
{
    if(mem == NULL)
    {
        mem = malloc(sizeof(Mem));
        if(mem == NULL)
        {
            printf("error while allocating memory !\n");
            exit(1);
        }
        //
        //
        //
    }
    if(mem->address == address)
    {
        //
        //
        //
    }
    mem->next = write_mem(mem->next, address, value);
    return mem;
}
int read_mem(Mem *mem, int address)
{
    if(mem == NULL)
    {
        mem = malloc(sizeof(Mem));
        if(mem == NULL)
        {
            printf("error while allocating memory !\n");
            exit(1);
        }
        mem->address = address;
        mem->next = NULL;
        return mem->value;
    }
    if(mem->address == address)
        return mem->value;
    return read_mem(mem->next, address);
}
int cond_verified(int cond, int SR)
{
    switch(cond)
    {
    case 0x0:
        return !(SR & 0x1);
    case 0x1://////
    case 0x2://////
    case 0x3://////
    case 0x4://////
    case 0x5://////
    case 0x6://////
    case 0x7://////
        break;
    }
}
int bits2int_4(int val)
{
    if(val & 0x8)
        return (val & 0x7) | 0x80000000;
    return val;
}
int val8(int val, int pos)
{
    //
    //
    //
}
int set8(int val3, int val2, int val1, int val0)
{
    //
    //
    //
}
int bits2int_16(int val)
{
    if(val & 0x8000)
        return -(val & 0x7fff);
    return val;
}
int int2bits_16(int val)
{
    if(val < 0)
        return (-val) & 0x8000;
    return val;
}
int val16(int val, int pos)
{
    switch(pos)
    {
    case 0:
        return val & 0xffff;
    case 1:
        return (val >> 16) & 0xffff;
    }
}
int set16(int val1, int val0)
{
    int ret = 0;
    ret |= val1 & 0xffff;
    ret <<= 16;
    ret |= val0 & 0xffff;
    return ret;
}
void execute(State *s)
{
    int instr = read_mem(s->mem, s->PC);
    if(!cond_verified((instr >> 8) & 0x7, s->SR))
    {
        s->PC++;
        return;
    }
    int v01234567 = instr & 0xff;
    int v0123 = instr & 0xf;
    int v4567 = (instr >> 4) & 0xf;
    int v45 = (instr >> 4) & 0x3;
    int v67 = (instr >> 6) & 0x3;
    int v6 = (instr >> 6) & 0x1;
    int v7 = (instr >> 7) & 0x1;
    int v4 = (instr >> 4) & 0x1;
    int v0 = instr & 0x1;
    int sval = bits2int_4(v0123);
    int *iH = &(s->reg[v4567][1]);
    int *iL = &(s->reg[v4567][0]);
    int *jH = &(s->reg[v0123][1]);
    int *jL = &(s->reg[v0123][0]);
    int *AccL = &(s->reg[0][0]);
    int *AccH = &(s->reg[0][1]);
    int *i2H = &(s->reg[v45][1]);
    int *i2L = &(s->reg[v45][0]);
    int val, valHH, valHL, valLH, valLL, valHHH, valHHL, valHLH, valHLL, valLHH, valLHL, valLLH, valLLL;
    switch((instr >> 11) & 0x1f)
    {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
        //
        //
        //
    case 0x8:
        *iH += sval;
        if((long long)*iL + (long long)sval > INT_MAX || (long long)*iL + (long long)sval < INT_MIN)
            s->SR |= 0x4;
        if((long long)(unsigned)*iL + (long long)(unsigned)sval > INT_MAX)
            s->SR |= 0x8;
        *iL += sval;
        if(*iL == 0)
            s->SR |= 0x1;
        if(*iL < 0)
            s->SR |= 0x2;
        s->PC++;
        break;
    case 0x9:
    case 0xa:
    case 0xb:
    case 0xc:
    case 0xd:
        //
        //
        //
    case 0xe:
        valHL = bits2int_16(val16(*jH, 1)) + bits2int_16(val16(*jH, 0)) + bits2int_16(val16(*jL, 1)) + bits2int_16(val16(*jL, 0));
        valLL = bits2int_16(val16(*iH, 1)) + bits2int_16(val16(*iH, 0)) + bits2int_16(val16(*iL, 1)) + bits2int_16(val16(*iL, 0));
        if(valHL > 0x7fff || valHL < -0x7fff || valLL > 0x7fff || valLL < -0x7fff)
            s->SR |= 0x4;
        if(valLL == 0)
            s->SR |= 0x1;
        if(valLL < 0)
            s->SR |= 0x2;
        if(valHL > 0x7fff)
            valHL = 0x7fff;
        if(valHL < -0x7fff)
            valHL = -0x7fff;
        if(valLL > 0x7fff)
            valLL = 0x7fff;
        if(valLL < -0x7fff)
            valLL = -0x7fff;
        *iH = set16(val16(*iH, 1), int2bits_16(valHL));
        *iL = set16(val16(*iL, 1), int2bits_16(valLL));
        s->PC++;
        break;
    case 0xf:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1a:
    case 0x1b:
        //
        //
        //
    case 0x1c:
        if(v4 == 0)
        {
            write_mem(s->mem, s->SP, val16(*jL, 1));
            write_mem(s->mem, s->SP + 1, val16(*jL, 0));
            s->SP -= 2;
        }
        else
        {
            write_mem(s->mem, s->SP, val16(*jH, 1));
            write_mem(s->mem, s->SP + 1, val16(*jH, 0));
            s->SP -= 2;
        }
        s->PC++;
        break;
    case 0x1d:
    case 0x1e:
    case 0x1f:
        //
        //
        //
        break;
    }
}
int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Please provide a binary file to execute !\n");
        exit(1);
    }
    FILE *file = fopen(argv[1], "r");
    if(file == NULL)
    {
        printf("Error opening file: %s\n", argv[1]);
        exit(1);
    }
    State *s = malloc(sizeof(State));
    if(s == NULL)
    {
        printf("error while allocating memory !\n");
        exit(1);
    }
    s->mem = NULL;
    s->PC = 0;
    s->SP = 0x100000;
    s->RA = 0;
    s->SR = 0;
    int pos = 0;
    while(!feof(file))
    {
        unsigned char c1 = fgetc(file);
        unsigned char c2 = fgetc(file);
        if(feof(file))
            break;
        int instr = (c1 << 8) + c2;
        s->mem = write_mem(s->mem, pos, instr);
        pos++;
    }
    while(1)
        execute(s);
    fclose(file);
    return 0;
}
