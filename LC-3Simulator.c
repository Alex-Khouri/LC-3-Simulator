#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

void printState();
void fetch();
void setCC(int val);
void toIntArray(FILE* input, int output[]);
void orig();
void ld();
void lea();
void ldi();
void and();
void not();
void add();
void br();

int reg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int pc; // INCREMENT BY 1
int ir = 0;
char cc = 'Z';
int lines[52992]; // Number of user-accessable memory locations in LC-3
int line = 0;     // Relative value of PC (PC - ORIG)

int main(int argc, char *argv[])
{
    memset(lines, 0, 52992);
    FILE* file = fopen(argv[1], "r");
    toIntArray(file, lines);   // Convert input file into char array
    fclose(file);
    orig();    // Load initial memory location into PC
    fetch();
    int opcode = ir >> 12;
    while (opcode != 15) // i.e. HALT
    {
        switch(opcode)
        {
            case 2:
                ld();
                break;
            case 14:
                lea();
                break;
            case 10:
                ldi();
                break;
            case 5:
                and();
                break;
            case 9:
                not();
                break;
            case 1:
                add();
                break;
            case 0:
                br();
                break;
        }
		printf("after executing instruction\t0x%04x\n", ir);
        printState();
        fetch();
        opcode = ir >> 12;
    }
    return 0;
}

void printState()
{
    for (int i = 0; i < 8; i++)
    {
        printf("R%d\t0x%04x\n", i, reg[i]); // Print registers
    }
    printf("PC\t0x%04x\n", pc);
    printf("IR\t0x%04x\n", ir);
    printf("CC\t%c\n", cc);
    printf("==================\n");
    return;
}

void fetch()
{
    ir = lines[line];
    pc++;
    line++;
    return;
}

void setCC(int val)
{
    if (val == 0) cc = 'Z';
    else if ((val & 0x8000) != 0) cc = 'N';
    else cc = 'P';
    return;
}

void toIntArray(FILE* input, int output[])
{   // Assumes array is initialised and can be overwritten
    int line = 0;
    int leftBits = getc(input);
    int rightBits = getc(input);
    while (leftBits != EOF)
    {
        output[line] = (leftBits << 8) + rightBits;
        leftBits = getc(input);
        rightBits = getc(input);
        line++;
    }
    return;
}

void orig()
{
    pc = lines[line];
    line++;
    return;
}

void ld()
{
    int dr = (ir >> 9) & 7;
    int pcOffset = ir & 0x1ff;
    if (pcOffset > 255) pcOffset = ((~pcOffset) + 1) * -1; // PC Offset < 0
    reg[dr] = lines[line + pcOffset];
    setCC(reg[dr]);
    return;
}

void lea()
{
    int dr = (ir >> 9) & 7;
    int pcOffset = ir & 0x1ff;
    if (pcOffset > 255) pcOffset = ((~pcOffset) + 1) * -1; // PC Offset < 0
    reg[dr] = pc + pcOffset;
    setCC(reg[dr]);
    return;
}

void ldi()
{
    int dr = (ir >> 9) & 7;
    int pcOffset = ir & 0x1ff;
    if (pcOffset > 255) pcOffset = ((~pcOffset) + 1) * -1;  // PC Offset < 0
    reg[dr] = lines[lines[line + pcOffset] - lines[0] + 1]; // lines[0] = ORIG
    setCC(reg[dr]); // +1 is necessary because lines[0] isn't first instruction
    return;
}

void and()
{
    int dr = (ir >> 9) & 7;
    int sr1 = (ir >> 6) & 7;
    if ((ir & 0x20) == 0)
    {
        int sr2 = (ir & 7);
        reg[dr] = (reg[sr1] & reg[sr2]) & 0xffff;
    }
    else
    {
        int imm = (ir & 0x1f);
        if (imm > 15) imm = ((~imm) + 1) * -1;  // imm < 0
        reg[dr] = (reg[sr1] & imm) & 0xffff;
    }
    setCC(reg[dr]);
    return;
}

void not()
{
    int dr = (ir >> 9) & 7;
    int sr = (ir >> 6) & 7;
    reg[dr] = (~reg[sr]) & 0xffff;
    setCC(reg[dr]);
    return;
}

void add()
{
    int dr = (ir >> 9) & 7;
    int sr1 = (ir >> 6) & 7;
    if ((ir & 0x20) == 0)
    {
        int sr2 = (ir & 7);
        reg[dr] = (reg[sr1] + reg[sr2]) & 0xffff;
    }
    else
    {
        int imm = (ir & 0x1f);
        if (imm > 15) imm = ((~imm) + 1) * -1;  // imm < 0
        reg[dr] = (reg[sr1] + imm) & 0xffff;
    }
    setCC(reg[dr]);
    return;
}

void br()
{
    int n = (ir >> 11) & 1;
    int z = (ir >> 10) & 1;
    int p = (ir >> 9) & 1;
    if (n && cc == 'N' || z && cc == 'Z' || p && cc == 'P')
    {
        int pcOffset = ir & 0x1ff;
        if (pcOffset > 255) pcOffset = ((~pcOffset) + 1) * -1; // PC Offset < 0
        pc += pcOffset;
        line += pcOffset;
    }

    return;
}