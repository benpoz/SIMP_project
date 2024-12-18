# include <stdio.h>
# include <string.h>


// import 

// convert text instruction line to number 
// bits need to be knowm beforehand
int hexToInt(char number[], int bits) { 
    number += 2;
    int res = 0;
    for(int i = 0; i < bits/4; i++) {
        int shamt = bits - 4*(i + 1);
        if (number[i] >= 'a' & number[i] <= 'f') {
            res += (number[i] - 87) << shamt;
        } else if (number[i] >= 'A' & number[i] <= 'F') {
            res += (number[i] - 55) << shamt;
        } else if (number[i] >= '0' & number[i] <= '9'){
            res += (number[i] - 48) << shamt;
        } else return -1;
    };
return res;
};

int isHex(char number[]) {
    return number[0] == '0' && (number[1] == 'x' | number[1] == 'X');
};

// create program counter
int PC = 0;

// create registers
int registers[16] = {0}; // set all to zero?

// create IOregisters
int IOregisters[22];

int monitor[256][256];
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

// decode instruction by shifting right then turn off all non relevant bits 
void decodeInstruction (ins) {
    struct instruction curr;
    curr.op_code = ins >> 40 && 0xff;
    curr.Rd = ins >> 36 && 0xf;
    curr.Rs = ins >> 32 && 0xf;
    curr.Rt = ins >> 28 && 0xf;
    curr.Rm = ins >> 24 && 0xf;
    curr.imm1 = ins >> 12 && 0xfff;
    curr.imm2 = ins && 0xfff;
};
void setImmediates (struct instruction ins) {
    registers[0] = 0; // just making sure
    registers[1] = ins.imm1;
    registers[2] = ins.imm2;         
};

// define operation by opcode
int execute (struct instruction ins);

int main(int argc, char *argv[]) { 
    if (argc != 14) { //check that the correct number of files was written in the command line
        printf("Wrong number of I/O files! need exactly 13 file names");
        return -1;
    };
    // input files
    FILE *instruction_memory = fopen(argv[1], "r");
    FILE *data_memory = fopen(argv[2], "w+");
    FILE *disk_in = fopen(argv[3], "w+");
    FILE *irq2_in = fopen(argv[4], "r");

    // output files
    FILE *data_memory_out = fopen(argv[5], "w+"); // how to make?
    FILE *registers_out = fopen(argv[6], "w+"); // remember to not output regs 0,1,2
    FILE *trace = fopen(argv[7], "w+");
    FILE *hwreg_trace = fopen(argv[8], "w+");
    FILE *cycles = fopen(argv[9], "w+");
    FILE *leds = fopen(argv[10], "w+");
    FILE *display7seg = fopen(argv[11], "w+");
    FILE *disk_out = fopen(argv[12], "w+"); // how to make?
    FILE *monitor = fopen(argv[13], "w+");

    while (!feof(instruction_memory)) {
        // read line by line and append to the existing array
    };

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
                registers[ins.Rd] = registers[ins.Rs] & registers[ins.Rt] & registers[ins.Rm];
                break;
            case 4: // or
                registers[ins.Rd] = registers[ins.Rs] | registers[ins.Rt] | registers[ins.Rm];
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
                if (registers[ins.Rs] == registers[ins.Rd]) {
                    PC = registers[ins.Rm] & 0xfff;
                }
                break;
            case 10: // bne
                if (registers[ins.Rs] != registers[ins.Rd]) {
                    PC = registers[ins.Rm] & 0xfff;
                }
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