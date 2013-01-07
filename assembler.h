typedef struct Labels {
  char *label;
  int address;
  struct Labels *next;
} Labels;

typedef struct Data {
  char *label;
  int num;
  struct Data *next;
} Data;
