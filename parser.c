#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#define MAX_LINE 256
#define MAX_WORD 256
int is_dec(char *word)
{
  if(strlen(word) <= 0 || word[0] == '-' && strlen(word) <= 1)
    return 0;
  int i = 0;
  if(word[i] == '-')
    i++;
  for(; i < strlen(word); i++)
    if(!isdigit(word[i]))
      return 0;
  return 1;
}
int parse_dec(char *word)
{
  int ret = 0;
  int neg = 0;
  int i = 0;
  if(word[i] == '-')
    {
      neg = 1;
      i++;
    }
  for(; i < strlen(word); i++)
    {
      ret *= 10;
      ret += word[i] - '0';
    }
  if(neg)
    ret = -ret;
  return ret;
}
int is_hex(char *word)
{
  if(strlen(word) <= 2 || word[0] == '-' && strlen(word) <= 3)
    return 0;
  int i = 0;
  if(word[i] == '-')
    i++;
  if(word[i] != '0')
    return 0;
  i++;
  if(word[i] != 'x')
    return 0;
  i++;
  for(; i < strlen(word); i++)
    if(!isxdigit(word[i]))
      return 0;
  return 1;
}
int parse_hex(char *word)
{
  int ret = 0;
  int neg = 0;
  int i = 0;
  if(word[i] == '-')
    {
      neg = 1;
      i++;
    }
  i += 2;
  for(; i < strlen(word); i++)
    {
      ret *= 16;
      if(isdigit(word[i]))
	ret += word[i] - '0';
      if(islower(word[i]))
	ret += word[i] - 'a' + 10;
      if(isupper(word[i]))
	ret += word[i] - 'A' + 10;
    }
  if(neg)
    ret = -ret;
  return ret;
}
int is_num(char *word)
{
  return is_dec(word) || is_hex(word);
}
int parse_num(char *word)
{
  int ret;
  if(is_dec(word))
    ret = parse_dec(word);
  if(is_hex(word))
    ret = parse_hex(word);
  return ret;
}
int is_reg(char *word)
{
  if(strlen(word) <= 0)
    return 0;
  if(word[0] != '$')
    return 0;
  return is_dec(word + 1);
}
int parse_reg(char *word)
{
  return parse_dec(word + 1);
}
int is_ident(char *word)
{
  if(strlen(word) <= 0)
    return 0;
  if((word[0] < 'a' || word[0] > 'z') && (word[0] < 'A' || word[0] > 'Z') && word[0] != '_')
    return 0;
  int i;
  for(i = 0; i < strlen(word); i++)
    if((word[0] < 'a' || word[0] > 'z') && (word[0] < 'A' || word[0] > 'Z') && (word[0] < '0' || word[0] > '9') && word[0] != '_')
      return 0;
  return 1;
}
char *parse_ident(char *word)
{
  char *ret = malloc((strlen(word) + 1) * sizeof(char));
  if(ret == NULL)
    {
      printf("error while allocating memory !\n");
      exit(1);
    }
  strcpy(ret, word);
  return ret;
}
int is_pos(char *word)
{
  Pos pos;
  for(pos = 0; pos <= MAX_POS; pos++)
    if(strcmp(word, pos_get_string(pos)) == 0)
      return 1;
  return 0;
}
int parse_pos(char *word)
{
  Pos pos;
  for(pos = 0; pos <= MAX_POS; pos++)
    if(strcmp(word, pos_get_string(pos)) == 0)
      return pos;
}
int is_label(char *word)
{
  if(word[strlen(word) - 1] != ':')
    return 0;
  char word2[MAX_WORD];
  strcpy(word2, word);
  word2[strlen(word2) - 1] = '\0';
  return is_ident(word2);
}
char *parse_label(char *word)
{
  char word2[MAX_WORD];
  strcpy(word2, word);
  word2[strlen(word2) - 1] = '\0';
  return parse_ident(word2);
}
int is_opcode(char *word)
{
  OpCode code;
  for(code = 0; code <= MAX_OP_CODE; code++)
    if(strcmp(word, opcode_get_string(code)) == 0)
      return 1;
  return 0;
}
OpCode parse_opcode(char *word)
{
  OpCode code;
  for(code = 0; code <= MAX_OP_CODE; code++)
    if(strcmp(word, opcode_get_string(code)) == 0)
      return code;
}
int is_cond(char *word)
{
  Cond cond;
  for(cond = 0; cond <= MAX_COND; cond++)
    if(strcmp(word, cond_get_string(cond)) == 0)
      return 1;
  return 0;
}
Cond parse_cond(char *word)
{
  Cond cond;
  for(cond = 0; cond <= MAX_OP_CODE; cond++)
    if(strcmp(word, cond_get_string(cond)) == 0)
      return cond;
}
int is_argument(char *word)
{
  return is_num(word) || is_reg(word) || is_ident(word) || is_pos(word);
}
Arguments *parse_argument(char *word)
{
  Arguments *ret = malloc(sizeof(Arguments));
  if(ret == NULL)
    {
      printf("error while allocating memory !\n");
      exit(1);
    }
  ret->previous = NULL;
  if(is_num(word))
    {
      ret->type = NUM;
      ret->num = parse_num(word);
    }
  if(is_reg(word))
    {
      ret->type = REG;
      ret->num = parse_reg(word);
    }
  if(is_ident(word))
    {
      ret->type = IDENT;
      ret->ident = parse_ident(word);
    }
  if(is_pos(word))
    {
      ret->type = POS;
      ret->pos = parse_pos(word);
    }
  return ret;
}
int get_arguments(char *line, char words[][MAX_WORD], int nwords)
{
  int pos_line = 0;
  int pos_word;
  int word;
  for(word = 0; word < nwords; word++)
    {
      pos_word = 0;
      for(; pos_line < strlen(line); pos_line++)
	if(!isspace(line[pos_line]))
	  break;
      for(; pos_line < strlen(line); pos_line++)
	{
	  if(line[pos_line] == ',')
	    {
	      pos_line++;
	      break;
	    }
	  else
	    words[word][pos_word] = line[pos_line];
	  pos_word++;
	}
      for(pos_word--; pos_word >= 0; pos_word--)
	if(!isspace(words[word][pos_word]))
	  break;
      words[word][pos_word + 1] = '\0';
    }
  for(word = 0; word < nwords; word++)
    if(strlen(words[word]) == 0)
      break;
  return word;
}
int is_arguments(char *word)
{
  int i;
  char args[4][MAX_WORD] = {"", "", "", ""};
  int nargs = get_arguments(word, args, 4);
  for(i = 0; i < nargs; i++)
    if(!is_argument(args[i]))
      return 0;
  return 1;
}
Arguments *parse_arguments(char *word)
{
  int i;
  Arguments *ret = NULL;
  char args[4][MAX_WORD] = {"", "", "", ""};
  int nargs = get_arguments(word, args, 4);
  for(i = 0; i < nargs; i++)
    {
      Arguments *tmp = parse_argument(args[i]);
      tmp->previous = ret;
      ret = tmp;
    }
  return ret;
}
int get_words(char *line, char words[][MAX_WORD], int nwords)
{
  int i;
  for(i = 0; i < strlen(line); i++)
    if(line[i] == '#' || line[i] == '\n')
      {
	line[i] = '\0';
	break;
      }
  int pos_line = 0;
  int pos_word;
  int word;
  for(word = 0; word < nwords - 1; word++)
    {
      pos_word = 0;
      for(; pos_line < strlen(line); pos_line++)
	if(!isspace(line[pos_line]))
	  break;
      for(; pos_line < strlen(line); pos_line++)
	{
	  if(isspace(line[pos_line]))
	    break;
	  else
	    words[word][pos_word] = line[pos_line];
	  pos_word++;
	}
      words[word][pos_word] = '\0';
    }
  if(pos_line != strlen(line))
    strcpy(words[word], line + pos_line);
  for(word = 0; word < nwords; word++)
    if(strlen(words[word]) == 0)
      break;
  return word;
}
int is_line(char *line)
{
  char words[3][MAX_WORD] = {"", "", ""};
  int nwords = get_words(line, words, 3);
  if(nwords == 0)
    return 2;
  else if(nwords == 1)
    if(is_num(words[0]) || is_label(words[0]))
      return 1;
    else
      printf("Should be label, numeral or \"STOP\": %s\n", words[0]);
  else if(nwords >= 2)
    if(is_opcode(words[0]))
      if(is_cond(words[1]))
	if(is_arguments(words[2]))
	  return 1;
	else
	  printf("Should be arguments list: %s\n", words[2]);
      else
	printf("Should be condition: %s\n", words[1]);
    else
      printf("Should be opcode: %s\n", words[0]);
  printf("Unrecognized line: %s\n", line);
  return 0;
}
Program *parse_line(char *line)
{
  Program *ret = malloc(sizeof(Program));
  if(ret == NULL)
    {
      printf("error while allocating memory !\n");
      exit(1);
    }
  ret->previous = NULL;
  char words[3][MAX_WORD] = {"", "", ""};
  int nwords = get_words(line, words, 3);
  if(nwords == 0)
    {
      free(ret);
      ret = NULL;
      return ret;
    }
  if(nwords == 1)
    if(is_num(words[0]))
      {
	ret->type = DATA;
	ret->num = parse_num(words[0]);
      }
      else if(is_label(words[0]))
	{
	  ret->type = LABEL;
	  ret->label = parse_label(words[0]);
	}
  if(nwords >= 2)
    {
      ret->type = INSTRUCTION;
      ret->instr = malloc(sizeof(Instruction));
      if(ret->instr == NULL)
	{
	  printf("error while allocating memory !\n");
	  exit(1);
	}
      ret->instr->op = parse_opcode(words[0]);
      ret->instr->cond = parse_cond(words[1]);
      ret->instr->args = parse_arguments(words[2]);
    }
  return ret;
}
Program *parse_program(FILE *file)
{
  Program *ret = NULL;
  while(!feof(file))
    {
      char line[MAX_LINE];
      if(fgets(line, MAX_LINE, file) == NULL)
	line[0] = '\0';
      if(is_line(line) == 1)
	{
	  Program *tmp = parse_line(line);
	  tmp->previous = ret;
	  ret = tmp;
	}
    }
  return ret;
}
void free_arguments(Arguments *args)
{
  if(args == NULL)
    return;
  free_arguments(args->previous);
  if(args->type == IDENT)
    free(args->ident);
}
void free_instruction(Instruction *instr)
{
  if(instr == NULL)
    return;
  free_arguments(instr->args);
}
void free_program(Program *prog)
{
  if(prog == NULL)
    return;
  free_program(prog->previous);
  free_instruction(prog->instr);
  if(prog->type == LABEL)
    free(prog->label);
}
int count_lines(Program *prog)
{
  if(prog == NULL)
    return 0;
  return 1 + count_lines(prog->previous);
}
int count_args(Arguments *args)
{
  if(args == NULL)
    return 0;
  return 1 + count_args(args->previous);
}
