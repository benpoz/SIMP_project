#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// define file sizes
#define MEMORY_SIZE 4096        // Maximum number of lines in the memory
#define LINE_LENGTH 14          // Each line can hold 12 characters + 2 for newline
#define DISK_SIZE 16384           // Number of bytes in the disk
#define MAX_CYCLES (1024*4096)  // Maximum possible cycles needed to execute a program

// create program counter & clock
unsigned int CLK = 1; 
unsigned int PC = 0;

// create registers
int registers[16] = {0}; // set all to zero

// create IOregisters
int IOregisters[23] = {0};
unsigned char monitor[256][256] = {0}; // unsigned char <- every pixel value is a byte
char IOregisters_names[23][12] = {
    "irq0enable",
    "irq1enable",
    "irq2enable",
    "irq0status",
    "irq1status",
    "irq2status",
    "irqhandler",
    "irqerturn",
    "clks",
    "leds",
    "display7seg",
    "timerenable",
    "timercurrent",
    "timermax",
    "diskcmd",
    "disksector",
    "diskbuffer",
    "diskstatus",
    "reserved1",
    "reserved2",
    "monitoraddr",
    "monitordata",
    "monitorcmd"
};
// disk operation counter
int disk_timer = 0;
int disk_timer_enable = 0;
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
long long int hexToNum(char number[], int bits); // should be signed or unsigned?
int write_file_contents_into_array(char* input_file_name, char** array, int max_lines, int max_line_length);
int write_integers_into_array(char* input_file_name, int* array, int max_lines);
long long int* createLongArrayFromFile (char* input_file_name, int max_lines, int max_line_length, int bits);
void decode_instruction(long long int ins, struct instruction *curr);
void setImmediates(struct instruction *ins);
int execute(struct instruction *ins, long long int *data_memory, long long int *disk_in, FILE* hwtrace, FILE* leds, FILE* disp7seg);
int countLinesToPrint (long long int *array, int max_size);
void processOutput(int outreg, struct instruction *ins);

int main(int argc, char *argv[]) {
    
    //sanity check
    printf("Simulator is running...\n\n");
    
    // check that the correct number of files was written in the command line
    if (argc != 15) { 
        printf("Wrong number of I/O files! need exactly 4 inputs & 10 outputs");
        return -1;
    } 
    
    //inputs
    long long int* instruction_memory = createLongArrayFromFile(argv[1], MEMORY_SIZE, LINE_LENGTH, 48);
    // get the line count so PC doesn't overflow
    int instruction_count = countLinesToPrint(instruction_memory, MEMORY_SIZE);     
    long long int* data_memory = createLongArrayFromFile(argv[2], MEMORY_SIZE, LINE_LENGTH, 32);
    long long int* disk_in = createLongArrayFromFile(argv[3], DISK_SIZE, LINE_LENGTH, 32);
    //interrupt2
    int irq2_in[MEMORY_SIZE];
    int times_interrupted = write_integers_into_array(argv[4], irq2_in, MEMORY_SIZE);
    
    //outputs
    FILE* memory_out = fopen(argv[5], "w");
    FILE* reg_out = fopen(argv[6], "w");
    FILE* trace = fopen(argv[7], "w");
    FILE* hwregtrace = fopen(argv[8], "w");
    FILE* cycles = fopen(argv[9], "w");
    FILE* leds = fopen(argv[10], "w");
    FILE* disp7seg = fopen(argv[11], "w");
    FILE* disk_out = fopen(argv[12], "w");
    FILE* monitor_txt = fopen(argv[13], "w");
    FILE* monitor_yuv = fopen(argv[14], "w");
    
    //execution loop
    while (1) { //TODO need to check relative timing of each interrupt/timer
        
        // exit if PC reached the end of imemin
        if (PC >= instruction_count) {
            fprintf(cycles, "%d", CLK - 1); //write cycle number to file
            printf("Out of instrucions after %d cycles\n", CLK - 1);
            break;
        }
        
        // if disk finished (if busy) reset timer and set irq1
        if (disk_timer == 1024 && disk_timer_enable) {
            disk_timer = 0;
            disk_timer_enable = 0;
            IOregisters[17] = 0;
            IOregisters[4] = 1;
        }
        //! rest of interruption logic missing
        
        //prepare instruction for execution
        struct instruction *current_instruction = malloc(sizeof(struct instruction));
        decode_instruction(instruction_memory[PC], current_instruction);
        setImmediates(current_instruction);
        free(current_instruction);
        
        //add line to trace file  
        fprintf(trace, "%03X %012llX ", PC, instruction_memory[PC]);
        for (int i = 0; i < 15; i++) {fprintf(trace, "%08X ", registers[i]);}
        fprintf(trace, "%08X\n", registers[15]); //new line after printing everything
        
        //execute
        int halt = execute(current_instruction, data_memory, disk_in, hwregtrace, leds, disp7seg); 
        
        //TODO stuff to write after execution:

        //finish 
        if (halt) {
            fprintf(cycles, "%d", CLK); //write cycle number to file
            printf("Halted after %d cycles\n", CLK);
            break;
        }
        if (disk_timer_enable) {disk_timer++;} // increment disk timer when disk is used

        PC++;
        CLK++;
    }
    
    //write to end-of-run output files: 
    
    //data memory
    int lines_data = countLinesToPrint(data_memory, MEMORY_SIZE);
    for (int i = 0; i < lines_data; i++) {
        fprintf(memory_out, "%08X\n", data_memory[i]);
    }

    //disk
    int disk_lines = countLinesToPrint(disk_in, DISK_SIZE);
    for (int i = 0; i < disk_lines; i++) {
        fprintf(disk_out, "%08X\n", disk_in[i]);
    }
    
    //regout
    for (int i = 3; i < 16; i++) {
        fprintf(reg_out, "%08X\n", registers[i]);
    }

    //monitors
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            fprintf(monitor_txt, "%02X\n", monitor[i][j]);
            fprintf(monitor_yuv, "%02X\n", monitor[i][j]);
        }
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
    printf("Loaded %d lines from %s\n", line_count, input_file_name);
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

void decode_instruction(long long int ins, struct instruction *curr) {
    curr->op_code = (ins >> 40) & 0xff;
    curr->Rd = (ins >> 36) & 0xf;
    curr->Rs = (ins >> 32) & 0xf;
    curr->Rt = (ins >> 28) & 0xf;
    curr->Rm = (ins >> 24) & 0xf;
    curr->imm1 = (ins >> 12) & 0xfff;
    curr->imm2 = ins & 0xfff;
}

void setImmediates(struct instruction *ins) {
    registers[0] = 0; // just making sure
    registers[1] = ins->imm1;
    registers[2] = ins->imm2;       
}

int execute(struct instruction *ins, long long int *data_memory, long long int *disk_in, FILE* hwtrace, FILE* leds, FILE* disp7seg) { // define operation by opcode
    switch (ins->op_code) {
        case 0: // add
            registers[ins->Rd] = registers[ins->Rs] + registers[ins->Rt] + registers[ins->Rm];
            break;
        case 1: // sub
            registers[ins->Rd] = registers[ins->Rs] - registers[ins->Rt] - registers[ins->Rm];
            break;
        case 2: // mac
            registers[ins->Rd] = registers[ins->Rs] * registers[ins->Rt] + registers[ins->Rm];
            break;
        case 3: // and
            registers[ins->Rd] = registers[ins->Rs] && registers[ins->Rt] && registers[ins->Rm];
            break;
        case 4: // or
            registers[ins->Rd] = registers[ins->Rs] || registers[ins->Rt] || registers[ins->Rm];
            break;
        case 5: // xor
            registers[ins->Rd] = registers[ins->Rs] ^ registers[ins->Rt] ^ registers[ins->Rm];
            break;
        case 6: // sll
            registers[ins->Rd] = registers[ins->Rs] << registers[ins->Rt];
            break;
        case 7: // sra
            registers[ins->Rd] = registers[ins->Rs] >> registers[ins->Rt];
            break;
        case 8: // srl
            registers[ins->Rd] = (int)((unsigned int)registers[ins->Rs] >> registers[ins->Rt]);
            break;
        case 9: // beq
            if (registers[ins->Rs] == registers[ins->Rt]) PC = (registers[ins->Rm] & 0xfff) - 1; // -1 to negate PC++ at end of loop
            break;
        case 10: // bne
            if (registers[ins->Rs] != registers[ins->Rt]) PC = (registers[ins->Rm] & 0xfff) - 1;// -1 to negate PC++ at end of loop
            break;
        case 11: // blt
            if (registers[ins->Rs] < registers[ins->Rt]) PC = (registers[ins->Rm] & 0xfff) - 1; // -1 to negate PC++ at end of loop
            break;
        case 12: // bgt
            if (registers[ins->Rs] > registers[ins->Rt]) PC = (registers[ins->Rm] & 0xfff) - 1; // -1 to negate PC++ at end of loop
            break;
        case 13: // ble
            if (registers[ins->Rs] <= registers[ins->Rt]) PC = (registers[ins->Rm] & 0xfff) - 1; // -1 to negate PC++ at end of loop
            break;
        case 14: // bge
            if (registers[ins->Rs] >= registers[ins->Rt]) PC = (registers[ins->Rm] & 0xfff) - 1; // -1 to negate PC++ at end of loop
            break;
        case 15: // jal
            registers[ins->Rd] = PC + 1;
            PC = registers[(ins->Rm & 0xfff)]- 1; // -1 to negate PC++ at end of loop
            break;
        case 16: // lw
            registers[ins->Rd] = data_memory[registers[ins->Rs] + registers[ins->Rt]] + registers[ins->Rm];
            break;
        case 17: // sw
            data_memory[registers[ins->Rs] + registers[ins->Rt]] = registers[ins->Rd] + registers[ins->Rm];
            break;
        case 18: // reti
            PC = IOregisters[7];
            break;
        case 19: // in
            int inreg = registers[ins->Rs] + registers[ins->Rt];
            registers[ins->Rd] = IOregisters[inreg];
            
            //?
            if (inreg == 22) {IOregisters[inreg] = 0;} //! if monitorcmd is read change it to zero?
            //?

            // print read command to files
            fprintf(hwtrace, "%d READ %s %08X\n", CLK, IOregisters_names[inreg], registers[ins->Rd]);
            break;
        case 20: // out
            int outreg = registers[ins->Rs] + registers[ins->Rt];

            switch (outreg)
                {
                case 14: // operate disk
                    if (!IOregisters[17]) { // check disk status before operating
                        IOregisters[17] = 1;
                        IOregisters[14] = registers[ins->Rm];
                        int sector = IOregisters[15];
                        int buffer = IOregisters[16]; 
                        
                        if (IOregisters[14] == 1) { // read from disk
                            disk_timer_enable = 1; // start counting disk operation time
                            for (int i = 0; i < 128; i++) {
                                data_memory[buffer + i] = disk_in[128*sector + i]; // this is the DMA
                            }
                        }
                        else if (IOregisters[14] == 2) { // write to disk
                            disk_timer_enable = 1; // start counting disk operation time
                            for (int i = 0; i < 128; i++) {
                                disk_in[128*sector + i] = data_memory[buffer + i]; // this is the DMA
                            }
                        }
                    }
                    break;
                case 15: // update disk sector
                    if (!IOregisters[17]) {
                        IOregisters[outreg] = registers[ins->Rm];
                    }
                    break;
                case 16: // update disk buffer
                    if (!IOregisters[17]) {
                        IOregisters[outreg] = registers[ins->Rm];
                    }
                    break;
                case 22:
                    // !how do you set monitorcmd??
                    if(IOregisters[22]) { // if monitorcmd is on update pixel
                        int line = (IOregisters[20] >> 8) & 0xff; // bits 8-15 contains monitor line
                        int column = IOregisters[20] & 0xff; // bits 0-7 contains monitor column
                        monitor[line][column] = IOregisters[21]; // update correct pixel with monitordata
                    }
                    break;
                default:
                    IOregisters[outreg] = registers[ins->Rm];
                    break;
                } break;
            
            // print write command to files
            fprintf(hwtrace, "%d WRITE %s %08X\n", CLK, IOregisters_names[outreg], registers[ins->Rm]);
            if (outreg == 9) {
                fprintf(leds, "%d %08X", CLK,  IOregisters[outreg]);
            }
            if (outreg == 10) {
                fprintf(disp7seg, "%d %08X", CLK,  IOregisters[outreg]);
            }
            // what about disk?
            break;
        case 21: // halt
            return 1; 
            break;
    }
    return 0;
} // define operation by opcode

long long int* createLongArrayFromFile(char* input_file_name, int max_lines, int max_line_length, int bits) {
    char* file_text[max_lines];
    int line_count = write_file_contents_into_array(input_file_name, file_text, max_lines, max_line_length);
    long long int* arr = malloc(max_lines * sizeof(long long int));

    for (int i = 0; i < line_count; i++) {
        arr[i] = hexToNum(file_text[i], bits);
    }

    for (int i = 0; i < line_count; i++) {
        free(file_text[i]);
    }

    for (int i = line_count; i < max_lines; i++) { 
        arr[i] = 0; // pad zeros to fill max lines
    }

    return arr;
}

int countLinesToPrint(long long int* array, int max_size) {  
    long long int sum = 0;
    int non_zero_lines = 0;
    for (int i = 0; i < max_size; i++) {
        sum += array[i];
    }
    while (sum != 0) {
        sum -= array[non_zero_lines];
        non_zero_lines++;
    }
    return non_zero_lines;
}
