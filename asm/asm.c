#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1000
#define MAX_LABEL_LENGTH 50
#define MAX_LABELS 1000
#define INSTRUCTION_SIZE 4096
#define DATA_SIZE 4096

typedef struct {
    char name[MAX_LABEL_LENGTH];
    int address;
} Label;

Label labels[MAX_LABELS];
int label_count = 0;

char* instructions[INSTRUCTION_SIZE];
int instruction_count = 0;

int data_memory[DATA_SIZE] = {0};
int data_count = 0;

int get_register_number(char* reg) {
    if (strcmp(reg, "$zero") == 0) return 0;
    if (strcmp(reg, "$imm1") == 0) return 1;
    if (strcmp(reg, "$imm2") == 0) return 2;
    if (strcmp(reg, "$v0") == 0) return 3;
    if (strcmp(reg, "$a0") == 0) return 4;
    if (strcmp(reg, "$a1") == 0) return 5;
    if (strcmp(reg, "$a2") == 0) return 6;
    if (strcmp(reg, "$t0") == 0) return 7;
    if (strcmp(reg, "$t1") == 0) return 8;
    if (strcmp(reg, "$t2") == 0) return 9;
    if (strcmp(reg, "$s0") == 0) return 10;
    if (strcmp(reg, "$s1") == 0) return 11;
    if (strcmp(reg, "$s2") == 0) return 12;
    if (strcmp(reg, "$gp") == 0) return 13;
    if (strcmp(reg, "$sp") == 0) return 14;
    if (strcmp(reg, "$ra") == 0) return 15;
    return -1;
}

int get_opcode(char* mnemonic) {
    if (strcmp(mnemonic, "add") == 0) return 0;
    if (strcmp(mnemonic, "sub") == 0) return 1;
    if (strcmp(mnemonic, "mac") == 0) return 2;
    if (strcmp(mnemonic, "and") == 0) return 3;
    if (strcmp(mnemonic, "or") == 0) return 4;
    if (strcmp(mnemonic, "xor") == 0) return 5;
    if (strcmp(mnemonic, "sll") == 0) return 6;
    if (strcmp(mnemonic, "sra") == 0) return 7;
    if (strcmp(mnemonic, "srl") == 0) return 8;
    if (strcmp(mnemonic, "beq") == 0) return 9;
    if (strcmp(mnemonic, "bne") == 0) return 10;
    if (strcmp(mnemonic, "blt") == 0) return 11;
    if (strcmp(mnemonic, "bgt") == 0) return 12;
    if (strcmp(mnemonic, "ble") == 0) return 13;
    if (strcmp(mnemonic, "bge") == 0) return 14;
    if (strcmp(mnemonic, "jal") == 0) return 15;
    if (strcmp(mnemonic, "lw") == 0) return 16;
    if (strcmp(mnemonic, "sw") == 0) return 17;
    if (strcmp(mnemonic, "reti") == 0) return 18;
    if (strcmp(mnemonic, "in") == 0) return 19;
    if (strcmp(mnemonic, "out") == 0) return 20;
    if (strcmp(mnemonic, "halt") == 0) return 21;
    return -1;
}

void add_label(char* name, int address) {
    strcpy(labels[label_count].name, name);
    labels[label_count].address = address;
    label_count++;
}

int get_label_address(char* name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            return labels[i].address;
        }
    }
    return -1;
}

void handle_data_directive(char* line) {
    char* token = strtok(line, " \t\n");
    token = strtok(NULL, " \t\n");
    
    while (token != NULL) {
        data_memory[data_count++] = (int)strtol(token, NULL, 0);
        token = strtok(NULL, " \t\n");
    }
}

void first_pass(FILE* input) {
    char line[MAX_LINE_LENGTH];
    int current_address = 0;

    while (fgets(line, sizeof(line), input)) {
        char* token = strtok(line, " \t\n");// strtok splits string to somthing
        if (token == NULL || token[0] == '#') continue;

        if (strcmp(token, ".word") == 0) {
            handle_data_directive(line);
        } else if (token[strlen(token) - 1] == ':') {
            token[strlen(token) - 1] = '\0';
            add_label(token, current_address);
        } else {
            current_address++;
        }
    }
}

void second_pass(FILE* input, FILE* imemin, FILE* dmemin) {
    char line[MAX_LINE_LENGTH];
    char instruction[13];
    rewind(input);

    while (fgets(line, sizeof(line), input)) {
        char* token = strtok(line, " \t\n");
        if (token == NULL || token[0] == '#') continue; // Skip comments
        if (token[strlen(token) - 1] == ':') continue;  // Skip labels
        if (strcmp(token, ".word") == 0) continue;      // Skip .word directive

        int opcode = get_opcode(token);
        int rd = 0, rs = 0, rt = 0, rm = 0;
        int imm1 = 0, imm2 = 0;

        // Parse RD
        token = strtok(NULL, ", \t\n");
        if (token) rd = get_register_number(token);

        // Parse RS
        token = strtok(NULL, ", \t\n");
        if (token) rs = get_register_number(token);

        // Parse RT
        token = strtok(NULL, ", \t\n");
        if (token) rt = get_register_number(token);

        // Parse RM or Imm1
        token = strtok(NULL, ", \t\n");
        if (token) {
            if (token[0] == '$') {
                int reg = get_register_number(token);
                if (reg >= 0) {
                    rm = reg;  // Register detected
                } else {
                    printf("Error: Unknown register %s\n", token);
                    exit(1);
                }
            } else if (isdigit(token[0]) || token[0] == '-') {
                imm1 = atoi(token);  // Direct integer immediate
            } else {
                imm1 = atoi(token);
            }
        }

        // Parse Imm2
        token = strtok(NULL, ", \t\n");
        if (token) {
            if (isdigit(token[0]) || token[0] == '-') {
                imm2 = atoi(token);  // Immediate integer
            } else {
                int label_address = get_label_address(token);
                if (label_address == -1) {
                    printf("Error: Undefined label %s\n", token);
                    exit(1);
                }
                imm2 = label_address;
            }
        }

        // Debugging output to verify parsed fields
        printf("Opcode: %02X, RD: %01X, RS: %01X, RT: %01X, RM: %01X, Imm1: %03X, Imm2: %03X\n",
               opcode, rd, rs, rt, rm, imm1, imm2);

        // Format the instruction into the proper binary format
        sprintf(instruction, "%02X%01X%01X%01X%01X%03X%03X",
                opcode, rd, rs, rt, rm, imm1 & 0xFFF, imm2 & 0xFFF);
        fprintf(imemin, "%s\n", instruction);  // Write to imemin file
    }

    // Write data memory values to dmemin
    for (int i = 0; i < data_count; i++) {
        fprintf(dmemin, "%08X\n", data_memory[i]);
    }
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <input_file> <imemin_file> <dmemin_file>\n", argv[0]);
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    if (!input) {
        printf("Error opening input file\n");
        return 1;
    }

    FILE* imemin = fopen(argv[2], "w");
    if (!imemin) {
        printf("Error opening imemin file\n");
        fclose(input);
        return 1;
    }

    FILE* dmemin = fopen(argv[3], "w");
    if (!dmemin) {
        printf("Error opening dmemin file\n");
        fclose(input);
        fclose(imemin);
        return 1;
    }

    first_pass(input);
    second_pass(input, imemin, dmemin);

    fclose(input);
    fclose(imemin);
    fclose(dmemin);
    return 0;
}
