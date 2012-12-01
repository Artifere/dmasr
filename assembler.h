typedef struct Labels
{
    char *label;
    int address;
    struct Labels *next;
} Labels;
