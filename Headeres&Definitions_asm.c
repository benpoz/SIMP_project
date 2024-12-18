//"import" in c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
// definitions for our assumptions for the inputs
#define MAX_LINE_LENGTH 1000
#define MAX_LABEL_LENGTH 50
#define MAX_LABELS 1000
#define INSTRUCTION_SIZE 4096

// a struct for our labels and a counter for them
typedef struct {
    char name[MAX_LABEL_LENGTH];
    int address;
} Label;
Label labels[MAX_LABELS];
int label_count = 0;
char* instructions[INSTRUCTION_SIZE];
int instruction_count = 0;

