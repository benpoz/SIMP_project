# include <stdio.h>
# include <string.h>

// define file sizes
#define MEMORY_SIZE 4096        // Maximum number of lines in the memory
#define LINE_LENGTH 14          // Each line can hold 12 characters + 2 for newline
#define DISK_SIZE 128           // Number of sectors in the disk
#define SECTOR_SIZE 514         // Each line can hold 512 bytes + 2 for newline
#define MAX_CYCLES 1024*4096    // Maximum possible cycles in a program

// define functions
int hexToInt(char number[], int bits) { 
    number += 2;
    int res = 0;
    for(int i = 0; i < bits/4; i++) {
        int shamt = bits - 4*(i + 1);
        if (number[i] >= 'a' && number[i] <= 'f') {
            res += (number[i] - 87) << shamt;
        } else if (number[i] >= 'A' && number[i] <= 'F') {
            res += (number[i] - 55) << shamt;
        } else if (number[i] >= '0' && number[i] <= '9'){
            res += (number[i] - 48) << shamt;
        } else return -1;
    };
return res;
};

int isHex(char number[]) {
    return number[0] == '0' && (number[1] == 'x' || number[1] == 'X');
};

int write_file_contents_into_array(char* input_file_name, char array[MEMORY_SIZE][LINE_LENGTH]) {
    FILE *file = fopen(input_file_name, "r");
    int line_count = 0;
    char buffer[LINE_LENGTH + 1];
    while (fgets(buffer, LINE_LENGTH + 1, file) != NULL) { // read line to buffer until at the EOF
        strcpy(array[line_count], buffer); // copy line to array
        line_count++;
    }    
    fclose(file);
    return line_count;
}

void decodeInstruction (ins) {
    struct instruction curr;
    curr.op_code = ins >> 40 & 0xff;
    curr.Rd = ins >> 36 & 0xf;
    curr.Rs = ins >> 32 & 0xf;
    curr.Rt = ins >> 28 & 0xf;
    curr.Rm = ins >> 24 & 0xf;
    curr.imm1 = ins >> 12 & 0xfff;
    curr.imm2 = ins & 0xfff;
};

void setImmediates (struct instruction ins) {
    registers[0] = 0; // just making sure
    registers[1] = ins.imm1;
    registers[2] = ins.imm2;         
};

// define operation by opcode

// create program counter & clock
int CLK = 0;
int PC = 0;

// create registers
int registers[16] = {0}; // set all to zero?

// create IOregisters
int IOregisters[22];
unsigned char monitor[256][256];

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

int main(int argc, char *argv[]) { 
    if (argc != 14) { //check that the correct number of files was written in the command line
        printf("Wrong number of I/O files! need exactly 13 file names");
        return -1;
    };
    // input files:
    char instruction_memory[MEMORY_SIZE][LINE_LENGTH];  // This holds instruction memory by line
    int instruction_count = write_file_contents_into_array(argv[1], instruction_memory);
    
    char data_memory[MEMORY_SIZE][LINE_LENGTH]; // This holds data memory by line
    int data_count = write_file_contents_into_array(argv[2], data_memory);   

    char disk_in[DISK_SIZE][SECTOR_SIZE]; // This holds disk contents by sector
    int disk_contents = write_file_contents_into_array(argv[3], disk_in);   

    int irq2_in[MAX_CYCLES][32]; // These are interupt times by cycle
    int disk_contents = write_file_contents_into_array(argv[4], irq2_in); 

    // output files:
    FILE *data_memory_out = fopen(argv[5], "w+"); // how to make?
    FILE *registers_out = fopen(argv[6], "w+"); // remember to not output regs 0,1,2
    FILE *trace = fopen(argv[7], "w+");
    FILE *hwreg_trace = fopen(argv[8], "w+");
    FILE *cycles = fopen(argv[9], "w+");
    FILE *leds = fopen(argv[10], "w+");
    FILE *display7seg = fopen(argv[11], "w+");
    FILE *disk_out = fopen(argv[12], "w+"); // how to make?
    FILE *monitor_txt = fopen(argv[13], "w+");
    FILE *monitor_yuv = fopen(argv[14], "w+");

    int execute(struct instruction ins) {
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
                if (registers[ins.Rs] < registers[ins.Rd]) {
                    PC = registers[ins.Rm] & 0xfff;
                }
                break;
            case 12: // bgt
                if (registers[ins.Rs] > registers[ins.Rd]) {
                    PC = registers[ins.Rm] & 0xfff;
                }
                break;
            case 13: // ble
                if (registers[ins.Rs] <= registers[ins.Rd]) {
                    PC = registers[ins.Rm] & 0xfff;
                }
                break;
            case 14: // bge
                if (registers[ins.Rs] >= registers[ins.Rd]) {
                    PC = registers[ins.Rm] & 0xfff;
                }
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
                return 0; // halt the program by returning -1
                break;
            default:
                return -1; // invalid instruction
                break;
        }; 
    };

	return 0;
}