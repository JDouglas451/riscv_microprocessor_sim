#include "riscv64_testing.h"

int main() {
    TESTING_INIT;

    word instr = OPCODE(0110111) | RD(00110) | utype_immediate(5120);
    INSTR_ASSERT("lui");
    DISASM_ASSERT("0x00001337   lui x6, 0x1000");

    instr = OPCODE(0010011) | RD(01001) | FUNCT3(000) | RS1(00111) | itype_immediate(73);
    INSTR_ASSERT("addi");

    instr = OPCODE(0010011) | RD(01110) | FUNCT3(100) | RS1(01001) | itype_immediate(85);
    INSTR_ASSERT("xori");

    instr = OPCODE(0010011) | RD(01110) | FUNCT3(110) | RS1(00000) | itype_immediate(115);
    INSTR_ASSERT("ori");

    instr = OPCODE(0010011) | RD(01000) | FUNCT3(111) | RS1(00001) | itype_immediate(27);
    INSTR_ASSERT("andi");

    instr = OPCODE(0010011) | RD(01110) | FUNCT3(001) | RS1(01001) | itype_immediate(32);
    INSTR_ASSERT("slli");

    instr = OPCODE(0010011) | RD(01110) | FUNCT3(101) | RS1(01001) | itype_immediate(32);
    INSTR_ASSERT("srli");

    instr = OPCODE(0010011) | RD(01110) | FUNCT3(101) | RS1(01001) | itype_immediate(32) | (INSTR_SIGN >> 1);
    INSTR_ASSERT("srai");

    instr = OPCODE(0110011) | RD(01011) | FUNCT3(000) | RS1(00000) | RS2(10010) | FUNCT7(0000000);
    INSTR_ASSERT("add");

    // TODO: SUB+ tests

    instr = OPCODE(1110011) | RD(00000) | FUNCT3(000) | RS1(00000) | RS2(00001) | FUNCT7(0000000);
    INSTR_ASSERT("ebreak");

    instr = OPCODE(0000011) | FUNCT3(010) | RD(10000) | RS1 (00000) | itype_immediate(76);
    INSTR_ASSERT("lw");

    instr = OPCODE(0100011) | FUNCT3(010) | RS1(10000) | RS2(00000) | stype_immediate(76);
    INSTR_ASSERT("sw");

    // TODO: JAL+ tests

    instr = OPCODE(0011011) | RD(00010) | FUNCT3(000) | RS1(00000) | itype_immediate(-95);
    INSTR_ASSERT("addiw");

    instr = OPCODE(0000011) | RD(00010) | FUNCT3(011) | RS1(00000) | itype_immediate(67);
    INSTR_ASSERT("ld");
}
