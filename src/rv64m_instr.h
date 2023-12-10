#ifndef SIM_RV64M_INSTRUCTIONS
#define SIM_RV64M_INSTRUCTIONS

// The RV64M Standard Extension for Integer Multiplication and Division

// Simulator data structures and typedefs
#include "riscv_config.h"

#include "riscv64_cpu.h"
typedef struct riscv64_cpu riscv_cpu_t;
dword cpu_read_register(const riscv_cpu_t* const cpu, int index);
void cpu_write_register(const riscv_cpu_t* const cpu, int index, dword value);

#include "riscv64_instr.h"
typedef struct riscv64_instruction_type riscv_instr_t;

// ---------- Disassembly and Execution Functions ----------

// Multiply (mul)
DISASM_DEF(mul) {
    return DISASM_FMT("mul %hhu, %hhu, %hhu", GET_RD, GET_RS1, GET_RS2);
}

EXEC_DEF(mul) {
    WRITE_REG(GET_RD, (sdword) GET_RS1 * (sdword) GET_RS2);
}

// TODO: mulh
// TODO: mulhsu
// TODO: mulhu
// TODO: div
// TODO: divu
// TODO: rem
// TODO: remu

// ---------- Word RV64M Instructions ----------

// TODO: mulw
// TODO: divw
// TODO: divuw
// TODO: remw
// TODO: remuw

// Array of all implemented rv64m instruction types
riscv_instr_t rv64m_instructions[] = {
    // Multiply (mul)
    {
        .mask = INSTR_OPCODE | INSTR_FUNCT3 | INSTR_FUNCT7,
        .required_bits = OPCODE(0110011) | FUNCT3(000) | FUNCT7(0000001),
        INSTR_LINKS(mul)
    },
};

// Number of implemented rv64m instructions
const size_t rv64m_size = sizeof(rv64m_instructions) / sizeof(riscv_instr_t);

#endif