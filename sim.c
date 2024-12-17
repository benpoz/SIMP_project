// import 

// convert text insctruction to number 
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

// import instructions (I/O missing!)
int instructions[4096];
// import data memory (I/O missing!)
int data_memory[4096];

// create program counter
int PC = 0;
// create registers
int registers[16]; // set all to zero?
registers[0] = 0;
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
}
void setImmediates (struct instruction ins) {
    registers[1] = ins.imm1;
    registers[2] = ins.imm2;         
};

// define operation by opcode
int execute (struct instruction ins) {
    switch (ins.op_code) // missing updating trace files
        {
        case 0: //add
            registers[ins.Rd] = registers[ins.Rs] + registers[ins.Rt] + registers[ins.Rm];
               break;
        case 1: //sub
            registers[ins.Rd] = registers[ins.Rs] - registers[ins.Rt] - registers[ins.Rm];
            break;
        case 2: //mac
            registers[ins.Rd] = registers[ins.Rs] * registers[ins.Rt] + registers[ins.Rm];
            break;
        case 3: //and
            registers[ins.Rd] = registers[ins.Rs] & registers[ins.Rt] & registers[ins.Rm];
            break;
        case 4: //or
            registers[ins.Rd] = registers[ins.Rs] | registers[ins.Rt] | registers[ins.Rm];
            break;
        case 5: //xor
            registers[ins.Rd] = registers[ins.Rs] ^ registers[ins.Rt] ^ registers[ins.Rm];
            break;
        case 6: //sll
            registers[ins.Rd] = registers[ins.Rs] << registers[ins.Rt];
            break;
        case 7: //sra
             registers[ins.Rd] = registers[ins.Rs] >> registers[ins.Rt];
        case 8: //srl
            registers[ins.Rd] =(int)((unsigned int)registers[ins.Rs] >> registers[ins.Rt]);
            break;
        case 9: //beq
            if (registers[ins.Rs] == registers[ins.Rd]) {PC = (registers[ins.Rm] && 0xfff);};
            break;
        case 10: //bne
            if (registers[ins.Rs] != registers[ins.Rd]) {PC = (registers[ins.Rm] && 0xfff);};
            break;
        case 11: //blt
           if (registers[ins.Rs] < registers[ins.Rd]) {PC = (registers[ins.Rm] && 0xfff);};
            break;
        case 12: //bgt
           if (registers[ins.Rs] > registers[ins.Rd]) {PC = (registers[ins.Rm] && 0xfff);};
            break;
        case 13: //ble
           if (registers[ins.Rs] <= registers[ins.Rd]) {PC = (registers[ins.Rm] && 0xfff);};
            break;
        case 14: //bge
           if (registers[ins.Rs] >= registers[ins.Rd]) {PC = (registers[ins.Rm] && 0xfff);};
            break;           
        case 15: //jal
            PC = registers[(ins.Rm && 0xfff)];
            registers[ins.Rd] = PC + 1;
            break;
        case 16: //lw
            registers[ins.Rd] = data_memory[ins.Rs + ins.Rt] + registers[ins.Rm];
            break;
        case 17: //sw
            data_memory[ins.Rs + ins.Rt] = registers[ins.Rd] + registers[ins.Rm];
            break;
        case 18: //reti
            PC = IOregisters[7];
            break;
        case 19: //in
            registers[ins.Rd] = IOregisters[ins.Rs + ins.Rt];
            break;
        case 20: //out
            IOregisters[ins.Rs + ins.Rt] = registers[ins.Rm];
            break;
        case 21: //halt
            return "halt"; // return something else
            break;
        default:
            return -1;
            break;
        }
}

