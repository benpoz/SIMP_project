#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// define file sizes
#define MEMORY_SIZE 4096        // Maximum number of lines in the memory
#define LINE_LENGTH 14          // Each line can hold 12 characters + 2 for newline
#define DISK_SIZE 16384           // Number of sectors in the disk
#define SECTOR_SIZE 514         // Each line can hold 512 bytes + 2 for newline
#define MAX_CYCLES (1024*4096)  // Maximum possible cycles needed to execute a program

// create program counter & clock
int CLK = 0;
int PC = 0;

// create registers
int registers[16] = {0}; // set all to zero

// create IOregisters
int IOregisters[22] = {0};
unsigned char monitor[256][256]; // unsigned char <- every pixel value is a byte

// define instruction structure
struct instruction {
    int op_code;
    int Rd;
    int Rs;
    int Rt;
    int Rm;
    int imm1;
    int imm2;
};

// declare functions
long long int hexToNum(char number[], int bits);
int write_file_contents_into_array(char* input_file_name, char** array, int max_lines, int max_line_length);
int write_integers_into_array(char* input_file_name, int* array, int max_lines);
void decode_instruction(long long int ins, struct instruction* curr);
void setImmediates(struct instruction ins);
int execute(struct instruction ins, int data_memory[]);

int main(int argc, char *argv[]) {
    printf("IM HERRREEE!\n");
    fflush(stdout);
    for (int i = 1; i < argc; i++) {
        printf("Argument %d: %s\n", i, argv[i]);
        fflush(stdout);
    }

    // if (argc != 14) { // check that the correct number of files was written in the command line
    //     printf("Wrong number of I/O files! need exactly 13 file names");
    //     return -1;
    // }
    // input files:
    char* instruction_memory_text[MEMORY_SIZE];
    int instruction_count = write_file_contents_into_array(argv[1], instruction_memory_text, MEMORY_SIZE, LINE_LENGTH);
    printf("Loaded %d lines from instruction memory file.\n", instruction_count);
    fflush(stdout);
    long long int* instruction_memory = malloc(instruction_count * sizeof(long long int));

    for (int i = 0; i < instruction_count; i++) {
        printf("Instrcution line %d is:%s", i + 1, instruction_memory_text[i]);
        instruction_memory[i] = hexToNum(instruction_memory_text[i], 48);
        printf("Instrcution line %d number is:%lld\n", i + 1, instruction_memory[i]);
    }

    char* data_memory_text[MEMORY_SIZE];
    int data_count = write_file_contents_into_array(argv[2], data_memory_text, MEMORY_SIZE, LINE_LENGTH);
    printf("Loaded %d lines from data memory file.\n", data_count);
    fflush(stdout);
    int* data_memory = malloc(data_count * sizeof(int));

    for (int i = 0; i < data_count; i++) {
        printf("Data line %d is:%s", i + 1, data_memory_text[i]);
        data_memory[i] = hexToNum(data_memory_text[i], 32);
        printf("Data line %d number is:%d\n", i + 1, data_memory[i]);
    }

    char* disk_in_text[DISK_SIZE];
    int disk_count = write_file_contents_into_array(argv[3], disk_in_text, DISK_SIZE, LINE_LENGTH);
    printf("Loaded %d lines from disk file.\n", disk_count);
    fflush(stdout);
    int* disk_in = malloc(disk_count * 128 * sizeof(int));
    
    for (int i = 0; i < disk_count; i++) {
        printf("Disk line %d is:%s", i + 1, disk_in_text[i]);
        disk_in[i] = hexToNum(disk_in_text[i], 32);
        printf("Disk line %d number is:%d\n", i + 1, disk_in[i]);
    }
    
    int irq2_in[MEMORY_SIZE];
    int times_interrupted = write_integers_into_array(argv[4], irq2_in, MEMORY_SIZE);
    printf("Loaded %d lines from interrupt file.\n", times_interrupted);
    fflush(stdout);

    for (int i = 0; i < times_interrupted; i++) {
        printf("Interrupt line %d is:%d\n", i + 1, irq2_in[i]);
    }
    
    // Clean up allocated memory
    for (int i = 0; i < instruction_count; i++) {
        free(instruction_memory_text[i]);
    }

    for (int i = 0; i < data_count; i++) {
        free(data_memory_text[i]);
    }

    for (int i = 0; i < disk_count; i++) {
        free(disk_in_text[i]);
    }

    return 0;
}

// define functions
long long int hexToNum(char number[], int bits) {
    if (number[0] == '0' && (number[1] == 'x' || number[1] == 'X')) {
        number += 2; // Skip the '0x' prefix
    }
    long long int res = 0;
    int len = strlen(number);
    
    while (len > 0 && (number[len - 1] == '\n' || number[len - 1] == ' ')) {
        len--;
    }

    for (int i = 0; i < len; i++) {
        int shamt = bits - 4 * (i + 1);
        char c = number[i];
        if (c >= '0' && c <= '9') {
            res += (long long int)(c - '0') << shamt;
        } else if (c >= 'a' && c <= 'f') {
            res += (long long int)(c - 'a' + 10) << shamt;
        } else if (c >= 'A' && c <= 'F') {
            res += (long long int)(c - 'A' + 10) << shamt;
        } else {
            return -1;
        }
    }
    return res;
}

int write_file_contents_into_array(char* input_file_name, char** array, int max_lines, int max_line_length) {
    FILE *file = fopen(input_file_name, "r");
    if (!file) {
        printf("Error opening file %s\n", input_file_name);
        return -1;
    }
    int line_count = 0;
    char buffer[max_line_length + 1];
    while (fgets(buffer, max_line_length + 1, file) != NULL && line_count < max_lines) {
        array[line_count] = strdup(buffer); // Allocate memory and copy the line
        line_count++;
    }
    fclose(file);
    return line_count;
}

int write_integers_into_array(char* input_file_name, int* array, int max_lines) {
    FILE *file = fopen(input_file_name, "r");
    if (!file) {
        printf("Error opening file %s\n", input_file_name);
        return -1;
    }
    int line_count = 0;
    char buffer[32];
    while (fgets(buffer, sizeof(buffer), file) != NULL && line_count < max_lines) {
        array[line_count] = atoi(buffer); // Allocate memory and copy the line
        line_count++;
    }
    fclose(file);
    return line_count;
}

void decode_instruction(long long int ins, struct instruction* curr) {
    curr->op_code = (ins >> 40) & 0xff;
    curr->Rd = (ins >> 36) & 0xf;
    curr->Rs = (ins >> 32) & 0xf;
    curr->Rt = (ins >> 28) & 0xf;
    curr->Rm = (ins >> 24) & 0xf;
    curr->imm1 = (ins >> 12) & 0xfff;
    curr->imm2 = ins & 0xfff;
}

void setImmediates (struct instruction ins) {
    registers[0] = 0; // just making sure
    registers[1] = ins.imm1;
    registers[2] = ins.imm2;         
}

int execute(struct instruction ins, int data_memory[]) { // define operation by opcode
    switch (ins.op_code) {
        case 0: // add
            registers[ins.Rd] = registers[ins.Rs] + registers[ins.Rt] + registers[ins.Rm];
            break;
        case 1: // sub
            registers[ins.Rd] = registers[ins.Rs] - registers[ins.Rt] - registers[ins.Rm];
            break;
        case 2: // mac
            registers[ins.Rd] = registers[ins.Rs] * registers[ins.Rt] + registers[ins.Rm];
            break;
        case 3: // and
            registers[ins.Rd] = registers[ins.Rs] && registers[ins.Rt] && registers[ins.Rm];
            break;
        case 4: // or
            registers[ins.Rd] = registers[ins.Rs] || registers[ins.Rt] || registers[ins.Rm];
            break;
        case 5: // xor
            registers[ins.Rd] = registers[ins.Rs] ^ registers[ins.Rt] ^ registers[ins.Rm];
            break;
        case 6: // sll
            registers[ins.Rd] = registers[ins.Rs] << registers[ins.Rt];
            break;
        case 7: // sra
            registers[ins.Rd] = registers[ins.Rs] >> registers[ins.Rt];
            break;
        case 8: // srl
            registers[ins.Rd] = (int)((unsigned int)registers[ins.Rs] >> registers[ins.Rt]);
            break;
        case 9: // beq
            if (registers[ins.Rs] == registers[ins.Rd]) PC = registers[ins.Rm] & 0xfff;
            break;
        case 10: // bne
            if (registers[ins.Rs] != registers[ins.Rd]) PC = registers[ins.Rm] & 0xfff;
            break;
        case 11: // blt
            if (registers[ins.Rs] < registers[ins.Rd]) PC = registers[ins.Rm] & 0xfff;
            break;
        case 12: // bgt
            if (registers[ins.Rs] > registers[ins.Rd]) PC = registers[ins.Rm] & 0xfff;
            break;
        case 13: // ble
            if (registers[ins.Rs] <= registers[ins.Rd]) PC = registers[ins.Rm] & 0xfff;
            break;
        case 14: // bge
            if (registers[ins.Rs] >= registers[ins.Rd]) PC = registers[ins.Rm] & 0xfff;
            break;
        case 15: // jal
            PC = registers[ins.Rm & 0xfff];
            registers[ins.Rd] = PC + 1;
            break;
        case 16: // lw
            registers[ins.Rd] = data_memory[registers[ins.Rs] + registers[ins.Rt]] + registers[ins.Rm];
            break;
        case 17: // sw
            data_memory[registers[ins.Rs] + registers[ins.Rt]] = registers[ins.Rd] + registers[ins.Rm];
            break;
        case 18: // reti
            PC = IOregisters[7];
            break;
        case 19: // in
            registers[ins.Rd] = IOregisters[registers[ins.Rs] + registers[ins.Rt]];
            break;
        case 20: // out
            IOregisters[registers[ins.Rs] + registers[ins.Rt]] = registers[ins.Rm];
            break;
        case 21: // halt
            return 1; // halt the program by returning 0
            break;
        default:
            return -1; // invalid instruction
            break;
    }
    return 0;
} // define operation by opcode