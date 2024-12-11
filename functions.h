// functions for use in both sim.c and asm.c
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

int hexToInt(char number[], int bits) {
    int res = 0;
    for(int i = 0; i < bits/4; i++) {
        int shamt = bits - 4*(i + 1);
        if (number[i] >= 'a' & number[i] <= 'F') {
            res += (number[i] - 87) << shamt;
        } else if (number[i] >= 'A' & number[i] <= 'F') {
            res += (number[i] - 55) << shamt;
        } else if (number[i] >= '0' & number[i] <= '9'){
            res += (number[i] - 48) << shamt;
        } else return -1;
    };
return res;
};

bool isHex(char number[]) {
    return number[0] == '0' && (number[1] == 'x' | number[1] == 'X');
};

#endif