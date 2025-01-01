// Required header files for the assembler
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Definitions for limits and sizes used in the assembler
#define MAX_LINE_LENGTH 1000
#define MAX_LABEL_LENGTH 50
#define MAX_LABELS 1000
#define INSTRUCTION_SIZE 4096
#define DATA_SIZE 4096

// Structure to represent a label with its name and memory address
typedef struct {
    char name[MAX_LABEL_LENGTH]; 
    int address;
} Label;

// Array to store all labels and a counter to track the number of labels
Label labels[MAX_LABELS];
int label_count = 0;

// Arrays and counters for instructions and data memory
char* instructions[INSTRUCTION_SIZE];
int instruction_count = 0;
int data_memory[DATA_SIZE] = {0};// Data memory initialized to zero
int highest_address = 0;  // Track highest address used

// Function to map register names to their numbers
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
    return -1; //Invalid register name
}

// Function to map instruction mnemonics to their opcodes
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
    return -1;// Invalid mnemonic
}
// Function to add a label and its address to the labels array
void add_label(char* name, int address) {
    strcpy(labels[label_count].name, name);// Store label name
    labels[label_count].address = address;// Store label address
    label_count++;// Increment label count
}
// Function to retrieve the address of a label by its name
int get_label_address(char* name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            return labels[i].address;// Return address if label is found
        }
    }
    return -1; // Label not found
}
// Function to handle .word directives and fill data memory
void handle_data_directive(char* line) {
    char* token = strtok(line, " \t\n");
    if (token == NULL || strcmp(token, ".word") != 0) return;// Ensure .word directive

    token = strtok(NULL, " \t\n");  // Extract address
    int address = 0;
    if (token) {
        address = (token[0] == '0' && token[1] == 'x') ? strtol(token, NULL, 16) : atoi(token);
    }

    token = strtok(NULL, " \t\n");  // Extract data
    int data = 0;
    if (token) {
        data = (token[0] == '0' && token[1] == 'x') ? strtol(token, NULL, 16) : atoi(token);
    }

    if (address >= 0 && address < DATA_SIZE) {
        data_memory[address] = data;// Store data in memory
        if (data != 0 && address > highest_address) {
            highest_address = address;  // Update highest address with non-zero value // Update highest non-zero address
        }
    } else {
        printf("Error: Address out of range (%d)\n", address);// Handle invalid address
        exit(1);
    }
}


// First pass to identify labels and handle .word directives
void first_pass(FILE* input) {
    char line[MAX_LINE_LENGTH];
    int current_address = 0; // Address counter

    while (fgets(line, sizeof(line), input)) {
        char line_copy[MAX_LINE_LENGTH];
        strcpy(line_copy, line);  // Make a copy of the line

        char* token = strtok(line, " \t\n");
        if (token == NULL || token[0] == '#') continue;// Ignore comments and empty lines

        if (strcmp(token, ".word") == 0) {
            handle_data_directive(line_copy);  // Use the copy for .word processing// Process .word directive 
        } else if (token[strlen(token) - 1] == ':') {
            token[strlen(token) - 1] = '\0';// Remove colon from label
            add_label(token, current_address);// Add label with its address
        } else {
            current_address++; // Increment address for each instruction
        }
    }
}


// Second pass to translate assembly code to machine code
void second_pass(FILE* input, FILE* imemin, FILE* dmemin) {
    char line[MAX_LINE_LENGTH];
    char instruction[13];// ?
    rewind(input);// Reset file pointer to the beginning

    while (fgets(line, sizeof(line), input)) {
        char* token = strtok(line, " \t\n");
        if (token == NULL || token[0] == '#') continue;// Skip comments and empty lines
        if (token[strlen(token) - 1] == ':') continue;// Skip label definitions
        if (strcmp(token, ".word") == 0) continue;// Skip .word directives

        int opcode = get_opcode(token);// Get opcode for the instruction
        int rd = 0, rs = 0, rt = 0, rm = 0;
        int imm1 = 0, imm2 = 0;

        token = strtok(NULL, ", \t\n");
        if (token) rd = get_register_number(token);// Destination register

        token = strtok(NULL, ", \t\n");
        if (token) rs = get_register_number(token);// first Source register

        token = strtok(NULL, ", \t\n");
        if (token) rt = get_register_number(token);// second Source register

        token = strtok(NULL, ", \t\n");
        if (token) rm = get_register_number(token);// third Source register

        token = strtok(NULL, ", \t\n");
        if (token) {
            if (isdigit(token[0]) || token[0] == '-') {
                imm1 = (token[0] == '0' && token[1] == 'x') ? strtol(token, NULL, 16) : atoi(token);// Immediate value 1
            } else {
                int label_address = get_label_address(token);// Get address of label?
                if (label_address == -1) {
                    printf("Error: Undefined label %s\n", token);
                    exit(1);
                }
                imm1 = label_address;
            }
        }

        token = strtok(NULL, ", \t\n");
        if (token) {
            if (isdigit(token[0]) || token[0] == '-') {
                imm2 = (token[0] == '0' && token[1] == 'x') ? strtol(token, NULL, 16) : atoi(token);// Immediate value 2
            } else {
                int label_address = get_label_address(token);
                if (label_address == -1) {
                    printf("Error: Undefined label %s\n", token);// Get address of label?
                    exit(1);
                }
                imm2 = label_address;
            }
        }
        // Format the instruction as a 12-character string?
        sprintf(instruction, "%02X%01X%01X%01X%01X%03X%03X", 
                opcode, rd, rs, rt, rm, imm1 & 0xFFF, imm2 & 0xFFF);
        fprintf(imemin, "%s\n", instruction);
    }

    // Write to dmemin up to highest_address// Write data memory to dmemin
    for (int i = 0; i <= highest_address; i++) {
        fprintf(dmemin, "%08X\n", data_memory[i]);
    }
}

// Main function to handle file operations and execute assembler passes
int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <input_file> <imemin_file> <dmemin_file>\n", argv[0]);
        return 1;
    }

    FILE* input = fopen(argv[1], "r");// Open input file
    if (!input) {
        printf("Error opening input file\n");
        return 1;
    }

    FILE* imemin = fopen(argv[2], "w");// Open instruction memory file
    if (!imemin) {
        printf("Error opening imemin file\n");
        fclose(input);
        return 1;
    }

    FILE* dmemin = fopen(argv[3], "w");// Open data memory file
    if (!dmemin) {
        printf("Error opening dmemin file\n");
        fclose(input);
        fclose(imemin);
        return 1;
    }

    first_pass(input);// Perform the first pass
    second_pass(input, imemin, dmemin);// Perform the second pass

    fclose(input);// Close input file
    fclose(imemin);// Close instruction memory file
    fclose(dmemin);// Close data memory file
    return 0;// Exit successfully
}



