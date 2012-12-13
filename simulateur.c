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
        mem->address = address; // On suppose qu'on ecrit dans la memoire dans l'ordre. Sinon, pour numeroter correctement
        //les cases memoires, il faudrait faire une fonction differente, qui se rappellerait de l'adresse de la case precedente
        mem->next = NULL;
    }

    if(mem->address == address)
        mem->value = value;
    else
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

    case 0x1:
        return SR & 0x1;

    case 0x2:
        return !(SR & 0x2);

    case 0x3:
        return (SR & 0x2) || (SR & 0x1);

    case 0x4:
        return SR & 0x2;

    case 0x5:
        return SR & 0x4;

    case 0x6:
        return SR & 0x8;

    case 0x7:
        return 1;
        break;
    }
}
int bits2int_4(int val)
{
    if(val & 0x8)
        return (val | 0xfffffff0);
    return val;
}
int val8(int val, int pos)
{
    switch(pos)
    {
    case 0:
        return val & 0xff;
    case 1:
        return (val >> 8) & 0xff;
    case 2:
        return (val >> 16) & 0xff;
    case 3:
        return (val >> 24) & 0xff;
    }
}
int set8(int val3, int val2, int val1, int val0)
{
    int ret = 0;
    ret |= val3 & 0xff;
    ret <<= 8;
    ret |= val2 & 0xff;
    ret <<= 8;
    ret |= val1 & 0xff;
    ret <<= 8;
    ret |= val0 & 0xff;

    return ret;
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
        return (-val) | 0x8000;
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
        *AccL &= 0xffffff00; //Ou alors juste un decalage ?
        *AccL += val8(v01234567, 0); //POS_LL
        s->PC++;
        break;

    case 0x1:
        s->PC++;
        break;

    case 0x2:
        if((long long)*iL + (long long)*jL > INT_MAX || (long long)*iL + (long long)*jL < INT_MIN)
            s->SR |= 0x8;
        if((long long)(unsigned)*iL + (long long)(unsigned)*jL > INT_MAX)
            s->SR |= 0x4;
        *iH += *jH;
        *iL += *jL;
        if (*iL == 0)
            s->SR |= 0x1;
        if (*iL < 0)
            s->SR |= 0x2;
        s->PC++;
        break;

    case 0x3:
        if((long long)*iL - (long long)*jL > INT_MAX || (long long)*iL - (long long)*jL < INT_MIN)
            s->SR |= 0x8;
        if((long long)(unsigned)*iL - (long long)(unsigned)*jL < 0)
            s->SR |= 0x4;
        *iH -= *jH;
        *iL -= *jL;
        if (*iL == 0)
            s->SR |= 0x1;
        if (*iL < 0)
            s->SR |= 0x2;
        s->PC++;
        break;

    case 0x4:
        *iH &= *jH;
        *iL &= *jL;
        if (!(*iL))
            s->SR |= 0x1;
        s->PC++;
        break;

    case 0x5:
        *iH |= *jH;
        *iL |= *jL;
        if (!(*iL))
            s->SR |= 0x1;
        s->PC++;
        break;



    case 0x6:
        *iH = !(*jH);
        *iL = !(*jL);
        if (!(*iL))
            s->SR |= 0x1;
        s->PC++;
        break;

    case 0x7:
        *iH ^= *jH;
        *iL ^= *jL;
        if (!(*iL))
            s->SR |= 0x1;
        s->PC++;
        break;

    case 0x8:
        *iH += sval;
        if((long long)*iL + (long long)sval > INT_MAX || (long long)*iL + (long long)sval < INT_MIN)
            s->SR |= 0x8;
        if((long long)(unsigned)*iL + (long long)(unsigned)sval > INT_MAX)
            s->SR |= 0x4;
        *iL += sval;
        if(*iL == 0)
            s->SR |= 0x1;
        if(*iL < 0)
            s->SR |= 0x2;
        s->PC++;
        break;

    case 0x9:
        if ( ((*iL) << ((unsigned) *jL))>>((unsigned) *jL) != *iL)
            s->SR |= 0x4;
        *iL <<= ((unsigned) *jL);
        *iH <<= ((unsigned) *jH);
        if (*iL == 0)
            s->SR |= 0x1;
        s->PC++;
        break;

    case 0xa:
        if ( ((*iL)>>((unsigned) *jL))<<((unsigned) *jL) != *iL)
            s->SR |= 0x4;
        *iL >>= ((unsigned) *jL);
        *iH >>= ((unsigned) *jH);
        if (*iL == 0)
            s->SR |= 0x1;
        s->PC++;
        break;

    case 0xb:
        *iL = set16(read_mem(s->mem, *jL), read_mem(s->mem, *jL+1));
        *iH = set16(read_mem(s->mem, *jL+2), read_mem(s->mem, *jL+3));
        if (*iL == 0)
            s->SR |= 0x1;
        s->PC++;
        break;

    case 0xc:
        s->mem = write_mem(s->mem, *iL, val16(*jL, 1));
        s->mem = write_mem(s->mem, *iL +1, val16(*jL, 0));
        s->mem = write_mem(s->mem, *iL +2, val16(*jH, 1));
        s->mem = write_mem(s->mem, *iL +3, val16(*jH, 0));
        if (*iL == 0)
            s->SR |= 0x1;
        s->PC++;
        break;



    case 0xd:
        s->PC += *iL;
        break;

    case 0xe:
        valHL = bits2int_16(val16(*jH, 1)) + bits2int_16(val16(*jH, 0)) + bits2int_16(val16(*jL, 1)) + bits2int_16(val16(*jL, 0));
        valLL = bits2int_16(val16(*iH, 1)) + bits2int_16(val16(*iH, 0)) + bits2int_16(val16(*iL, 1)) + bits2int_16(val16(*iL, 0));
        if(valHL > 0x7fff || valHL < -0x7fff || valLL > 0x7fff || valLL < -0x7fff)
            s->SR |= 0x8;
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
        valLL = bits2int_16(val16(*iL, 0)) - bits2int_16(val16(*jL, 0));
        valLH = bits2int_16(val16(*iL, 1)) - bits2int_16(val16(*jL, 1));
        valHL = bits2int_16(val16(*iH, 0)) - bits2int_16(val16(*jH, 0));
        valHH = bits2int_16(val16(*iH, 1)) - bits2int_16(val16(*jH, 1));
        if(valLL > 0x7fff || valLL < -0x7fff || valLH > 0x7fff || valLH < -0x7fff
                ||valHL > 0x7fff || valHL < -0x7fff || valHH > 0x7fff || valHH < -0x7fff)
            s->SR |= 0x8;
        if(valLL == 0 && valLH == 0)
            s->SR |= 0x1;
        if(valLL < 0 && valLH <0)
            s->SR |= 0x2;

        if(valLL > 0x7fff)
            valLL = 0x7fff;
        else if(valLL < -0x7fff)
            valLL = -0x7fff;
        if(valLH > 0x7fff)
            valLH = 0x7fff;
        else if(valLH < -0x7fff)
            valLH = -0x7fff;
        if(valHL > 0x7fff)
            valHL = 0x7fff;
        else if(valHL < -0x7fff)
            valHL = -0x7fff;
        if(valHH > 0x7fff)
            valHH = 0x7fff;
        else if(valHH < -0x7fff)
            valHH = -0x7fff;

        *iH = set16(int2bits_16(valHH), int2bits_16(valHL));
        *iL = set16(int2bits_16(valLH), int2bits_16(valLL));
        s->PC++;
        break;

    case 0x10:
        valLLL = val8(*iL, 0) + val8(*jL, 0);
        valLLH = val8(*iL, 1) + val8(*jL, 1);
        valLHL = val8(*iL, 2) + val8(*jL, 2);
        valLHH = val8(*iL, 3) + val8(*jL, 3);
        valHLL = val8(*iH, 0) + val8(*jH, 0);
        valHLH = val8(*iH, 1) + val8(*jH, 1);
        valHHL = val8(*iH, 2) + val8(*jH, 2);
        valHHH = val8(*iH, 3) + val8(*jH, 3);


        if(valLLL > 0x7f || valLLL < -0x7f || valLLH > 0x7f || valLLH < -0x7f
			  ||valLHL > 0x7f || valLHL < -0x7f || valLHH > 0x7f || valLHH < -0x7f
           ||valHLL > 0x7f || valHLL < -0x7f || valHLH > 0x7f || valHLH < -0x7f
			  ||valHHL > 0x7f || valHHL < -0x7f || valHHH > 0x7f || valHHH < -0x7f)
				s->SR |= 0x8;

        if(valLLL == 0 && valLLH == 0 && valLHL == 0 && valLHH == 0)
            s->SR |= 0x1;

        if(valLLL > 0x7f)
            valLLL = 0x7f;
        else if(valLLL < -0x7f)
            valLLL = -0x7f;
        if(valLLH > 0x7f)
            valLLH = 0x7f;
        else if(valLLH < -0x7f)
            valLLH = -0x7f;
        if(valLHL > 0x7f)
            valLHL = 0x7f;
        else if(valLHL < -0x7f)
            valLHL = -0x7f;
        if(valLHH > 0x7f)
            valLHH = 0x7f;
        else if(valLHH < -0x7f)
            valLHH = -0x7f;
        if(valHLL > 0x7f)
            valHLL = 0x7f;
        else if(valHLL < -0x7f)
            valHLL = -0x7f;
        if(valHLH > 0x7f)
            valHLH = 0x7f;
        else if(valHLH < -0x7f)
            valHLH = -0x7f;
        if(valHHL > 0x7f)
            valHHL = 0x7f;
        else if(valHHL < -0x7f)
            valHHL = -0x7f;
        if(valHHH > 0x7f)
            valHHH = 0x7f;
        else if(valHHH < -0x7f)
            valHHH = -0x7f;

        *iH = set8(valHHH, valHHL,valHLH, valHLL);
        *iL = set8(valLHH, valLHL,valLLH, valLLL);
        s->PC++;
        break;

    case 0x11:
        valLL = bits2int_16(val16(*iL, 0)) + bits2int_16(val16(*jL, 0));
        valLH = bits2int_16(val16(*iL, 1)) + bits2int_16(val16(*jL, 1));
        valHL = bits2int_16(val16(*iH, 0)) + bits2int_16(val16(*jH, 0));
        valHH = bits2int_16(val16(*iH, 1)) + bits2int_16(val16(*jH, 1));
        if(valLL > 0x7fff || valLL < -0x7fff || valLH > 0x7fff || valLH < -0x7fff
			  ||valHL > 0x7fff || valHL < -0x7fff || valHH > 0x7fff || valHH < -0x7fff)
            s->SR |= 0x8;
        if(valLL == 0 && valLH == 0)
            s->SR |= 0x1;
        if(valLL < 0 && valLH < 0)
            s->SR |= 0x2;

        if(valLL > 0x7fff)
            valLL = 0x7fff;
        else if(valLL < -0x7fff)
            valLL = -0x7fff;
        if(valLH > 0x7fff)
            valLH = 0x7fff;
        else if(valLH < -0x7fff)
            valLH = -0x7fff;
        if(valHL > 0x7fff)
            valHL = 0x7fff;
        else if(valHL < -0x7fff)
            valHL = -0x7fff;
        if(valHH > 0x7fff)
            valHH = 0x7fff;
        else if(valHH < -0x7fff)
            valHH = -0x7fff;

        *iH = set16(int2bits_16(valHH), int2bits_16(valHL));
        *iL = set16(int2bits_16(valLH), int2bits_16(valLL));
        s->PC++;
        break;

    case 0x12:
        valLL = bits2int_16(val16(*iL, 0)) * bits2int_16(val16(*jL, 0));
        valLH = bits2int_16(val16(*iL, 1)) * bits2int_16(val16(*jL, 1));
        valHL = bits2int_16(val16(*iH, 0)) * bits2int_16(val16(*jH, 0));
        valHH = bits2int_16(val16(*iH, 1)) * bits2int_16(val16(*jH, 1));
        if(valLL > 0x7fff || valLL < -0x7fff || valLH > 0x7fff || valLH < -0x7fff
			  ||valHL > 0x7fff || valHL < -0x7fff || valHH > 0x7fff || valHH < -0x7fff)
            s->SR |= 0x8;
        if(valLL == 0 && valLH == 0)
            s->SR |= 0x1;
        if(valLL < 0 && valLH <0)
            s->SR |= 0x2;

        if(valLL > 0x7fff)
            valLL = 0x7fff;
        else if(valLL < -0x7fff)
            valLL = -0x7fff;
        if(valLH > 0x7fff)
            valLH = 0x7fff;
        else if(valLH < -0x7fff)
            valLH = -0x7fff;
        if(valHL > 0x7fff)
            valHL = 0x7fff;
        else if(valHL < -0x7fff)
            valHL = -0x7fff;
        if(valHH > 0x7fff)
            valHH = 0x7fff;
        else if(valHH < -0x7fff)
            valHH = -0x7fff;

        *iH = set16(int2bits_16(valHH), int2bits_16(valHL));
        *iL = set16(int2bits_16(valLH), int2bits_16(valLL));
        s->PC++;
        break;

    case 0x13:
        valLL = bits2int_16(val16(*iL, 0)) + bits2int_16(val16(*iL, 1)) + bits2int_16(val16(*jL, 0)) + bits2int_16(val16(*jL, 1));
        valHL = bits2int_16(val16(*iH, 0)) + bits2int_16(val16(*iH, 1)) + bits2int_16(val16(*jH, 0)) + bits2int_16(val16(*jH, 1));

        if (valHL > 0x7fff || valHL < -0x7fff || valLL > 0x7fff || valLL < -0x7fff)
            s->SR |= 0x8;
        if(valLL == 0)
            s->SR |= 0x1;
        if(valLL < 0)
            s->SR |= 0x2;
        if(valHL > 0x7fff)
            valHL = 0x7fff;
        else if(valHL < -0x7fff)
            valHL = -0x7fff;
        if(valLL > 0x7fff)
            valLL = 0x7fff;
        else if(valLL < -0x7fff)
            valLL = -0x7fff;

        *iH = set16(val16(*iH, 1), int2bits_16(valHL));
        *iL = set16(val16(*iL, 1), int2bits_16(valLL));
        s->PC++;
        break;


    case 0x14:
        valLLL = val8(*iL, 0) - val8(*jL, 0);
        valLLH = val8(*iL, 1) - val8(*jL, 1);
        valLHL = val8(*iL, 2) - val8(*jL, 2);
        valLHH = val8(*iL, 3) - val8(*jL, 3);
        valHLL = val8(*iH, 0) - val8(*jH, 0);
        valHLH = val8(*iH, 1) - val8(*jH, 1);
        valHHL = val8(*iH, 2) - val8(*jH, 2);
        valHHH = val8(*iH, 3) - val8(*jH, 3);


        if(valLLL > 0x7f || valLLL < -0x7f || valLLH > 0x7f || valLLH < -0x7f
                ||valLHL > 0x7f || valLHL < -0x7f || valLHH > 0x7f || valLHH < -0x7f
                ||valHLL > 0x7f || valHLL < -0x7f || valHLH > 0x7f || valHLH < -0x7f
                ||valHHL > 0x7f || valHHL < -0x7f || valHHH > 0x7f || valHHH < -0x7f)
            s->SR |= 0x8;

        if(valLLL == 0 && valLLH == 0 && valLHL == 0 && valLHH == 0)
            s->SR |= 0x1;

        if(valLLL > 0x7f)
            valLLL = 0x7f;
        else if(valLLL < -0x7f)
            valLLL = -0x7f;
        if(valLLH > 0x7f)
            valLLH = 0x7f;
        else if(valLLH < -0x7f)
            valLLH = -0x7f;
        if(valLHL > 0x7f)
            valLHL = 0x7f;
        else if(valLHL < -0x7f)
            valLHL = -0x7f;
        if(valLHH > 0x7f)
            valLHH = 0x7f;
        else if(valLHH < -0x7f)
            valLHH = -0x7f;
        if(valHLL > 0x7f)
            valHLL = 0x7f;
        else if(valHLL < -0x7f)
            valHLL = -0x7f;
        if(valHLH > 0x7f)
            valHLH = 0x7f;
        else if(valHLH < -0x7f)
            valHLH = -0x7f;
        if(valHHL > 0x7f)
            valHHL = 0x7f;
        else if(valHHL < -0x7f)
            valHHL = -0x7f;
        if(valHHH > 0x7f)
            valHHH = 0x7f;
        else if(valHHH < -0x7f)
            valHHH = -0x7f;

        *iH = set8(valHHH, valHHL,valHLH, valHLL);
        *iL = set8(valLHH, valLHL,valLLH, valLLL);
        s->PC++;
        break;


    case 0x15:
        if (v67 == 0)
        {
            if (v45 == 0)
                *AccL = set16(val16(*AccL,1), val16(*jL, 0));
            if (v45 == 1)
                *AccL = set16(val16(*AccL,1), val16(*jL, 1));
            if (v45 == 2)
                *AccL = set16(val16(*AccL,1), val16(*jH, 0));
            if (v45 == 3)
                *AccL = set16(val16(*AccL,1), val16(*jH, 1));
            if (val16(*AccL, 0)==0)
                s->SR |= 0x1;
        }

        else if (v67 == 1)
        {
            if (v45 == 0)
                *AccL = set16(val16(*jL, 0), val16(*AccL,0));
            if (v45 == 1)
                *AccL = set16(val16(*jL, 1), val16(*AccL,0));
            if (v45 == 2)
                *AccL = set16(val16(*jH, 0), val16(*AccL,0));
            if (v45 == 3)
                *AccL = set16(val16(*jH, 1), val16(*AccL,0));
            if (val16(*AccL, 1)==0)
                s->SR |= 0x1;
        }

        else if (v67 == 2)
        {
            if (v45 == 0)
                *AccH = set16(val16(*AccH,1), val16(*jL, 0));
            if (v45 == 1)
                *AccH = set16(val16(*AccH,1), val16(*jL, 1));
            if (v45 == 2)
                *AccH = set16(val16(*AccH,1), val16(*jH, 0));
            if (v45 == 3)
                *AccH = set16(val16(*AccH,1), val16(*jH, 1));
            if (val16(*AccH, 0)==0)
                s->SR |= 0x1;
        }

        else if (v67 == 3)
        {
            if (v45 == 0)
                *AccH = set16(val16(*jL, 0), val16(*AccH,0));
            if (v45 == 1)
                *AccH = set16(val16(*jL, 1), val16(*AccH,0));
            if (v45 == 2)
                *AccH = set16(val16(*jH, 0), val16(*AccH,0));
            if (v45 == 3)
                *AccH = set16(val16(*jH, 1), val16(*AccH,0));
            if (val16(*AccH, 1)==0)
                s->SR |= 0x1;
        }
        else
        {
            printf("Invalid position in move16 arguments\n");
            exit(1);
        }
        s->PC++;
        break;

    case 0x16:
        if (v7 == 0)
        {
            if (v6 == 0)
                *i2L = *jL;
            if (v6 == 1)
                *i2L = *jH;
            if (*i2L == 0)
                s->SR |= 0x1;
        }

        else if (v7 == 1)
        {
            if (v6 == 0)
                *i2H = *jL;
            if (v6 == 1)
                *i2H = *jH;
            if (*i2H == 0)
                s->SR |= 0x1;
        }

        else
        {
            printf("Invalid position in move32 arguments\n");
            exit(1);
        }
        s->PC++;
        break;

    case 0x17:
        *iH = *jH;
        *iL = *jL;
        if (!(*jH) && !(*jL))
            s->SR |= 0x1;
        s->PC++;
        break;


    case 0x18:
        valLLL = ((int)s->SR & 0xf);
        *iL = *iL & (0xfffffff0);
        *iL |= valLLL;
        s->PC++;
        break;

    case 0x19:
        valLLL = (*jL) & 0xf;
        s->SR = valLLL;
        s->PC++;
        break;

    case 0x1a:
        if (*iL > *jL)
				*iL = 1;
        else
            *iL = 0;

        if (*iH > *jH)
            *iH = 1;
        else
            *iH = 0;

        if (*iL == 0)
            s->SR |= 0x1;
        s->PC++;
        break;

    case 0x1b:
        if (*iL <= *jL)
            *iL = 1;
        else
            *iL = 0;

        if (*iH <= *jH)
            *iH = 1;
        else
            *iH = 0;

        if (*iL == 0)
            s->SR |= 0x1;
        s->PC++;
        break;

    case 0x1c:
        if(v4 == 0)
        {
            s->mem = write_mem(s->mem, s->SP, val16(*jL, 1));
            s->mem = write_mem(s->mem, s->SP + 1, val16(*jL, 0));
            s->SP -= 2;
        }
        else if (v4 == 1)
        {
            s->mem = write_mem(s->mem, s->SP, val16(*jH, 1));
            s->mem = write_mem(s->mem, s->SP + 1, val16(*jH, 0));
            s->SP -= 2;
        }
        else
        {
            printf("Invalid position in push arguments\n");
            exit(1);
        }
        s->PC++;
        break;

    case 0x1d:
        s->SP += 2;
        if(v0 == 0)
        {
            *iL = set16(read_mem(s->mem, s->SP), read_mem(s->mem, s->SP+1));
            if (*iL == 0)
                s->SR |= 0x1;
        }
        else if (v0 == 1)
        {
            *iH = set16(read_mem(s->mem, s->SP), read_mem(s->mem, s->SP+1));
            if (*iH == 0)
                s->SR |= 0x1;
        }
        else
        {
            printf("Invalid position in pop arguments\n");
            exit(1);
        }
        s->PC++;
        break;

    case 0x1e:
        s->PC = s->RA;
        s->SP += 2;
        s->RA = set16(read_mem(s->mem, s->SP), read_mem(s->mem, s->SP+1));
        s->PC++;
        break;

    case 0x1f:
        s->mem = write_mem(s->mem, s->SP, val16(s->RA, 1));
        s->mem = write_mem(s->mem, s->SP + 1, val16(s->RA, 0));
        s->SP-=2;
        s->RA = s->PC;
        s->PC = *iL;
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
