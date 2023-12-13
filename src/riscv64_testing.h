#ifndef SIM_RISCV64_TESTING
#define SIM_RISCV64_TESTING

#include <string.h>
#include <stdio.h>

#include "rskapi.h"
#include "riscv64.h"

// ---------- Testing Setup and Assertions ----------

#define TESTING_BUFFER_SIZE 80
#define TESTING_INIT riscv_cpu_t* cpu = cpu_init(NULL, &test_services); if (NULL == cpu) return 1; char buffer[TESTING_BUFFER_SIZE]; size_t buffer_size = TESTING_BUFFER_SIZE;

#define INSTR_ASSERT(str_a) if (NULL == str_a || NULL == cpu_identify_instr(cpu, instr) || 0 != strncmp(str_a, cpu_identify_instr(cpu, instr), sizeof(str_a))) { fprintf(stderr, "Match failed: '%s' (evaluated to '%s')", str_a, cpu_identify_instr(cpu, instr)); }

#define DISASM_ASSERT(str_a) cpu_disassemble_instr(cpu, buffer, buffer_size, instr); if (NULL == str_a || 0 != strncmp(str_a, buffer, sizeof(str_a))) { fprintf(stderr, "Disassembly failed\nExpected: '%s'\nResult:   '%s'\n", str_a, buffer); }

// ---------- Preprocessor Bit Mask Construction ----------

#define BITSMASK(high, low) ((__UINT64_MAX__ << (high + 1)) ^ (__UINT64_MAX__ << low))
#define BITMASK(bit)        BITSMASK(bit, bit)

// ---------- Intruction Preprocessor Shortcuts ----------

#define INSTR_SIGN   BITMASK(31)

#define INSTR_FUNCT7 BITSMASK(31, 25)
#define FUNCT7(bits) ((0b##bits << 25) & INSTR_FUNCT7)

#define INSTR_RS2    BITSMASK(24, 20)
#define RS2(bits)    ((0b##bits << 20) & INSTR_RS2)

#define INSTR_RS1    BITSMASK(19, 15)
#define RS1(bits)    ((0b##bits << 15) & INSTR_RS1)

#define INSTR_FUNCT3 BITSMASK(14, 12)
#define FUNCT3(bits) ((0b##bits << 12) & INSTR_FUNCT3)

#define INSTR_RD     BITSMASK(11, 7)
#define RD(bits)     ((0b##bits << 7) & INSTR_RD)

#define INSTR_OPCODE BITSMASK(6, 0)
#define OPCODE(bits) (0b##bits & INSTR_OPCODE)

// ---------- Immediate Encoding Functions ----------

const word itype_immediate(const sword value) {
    return (value << 20);
}

const word stype_immediate(const sword value) {
    return ((BITSMASK(11, 5) & value) << 20) | ((BITSMASK(4, 0) & value) << 6);
}

const word btype_immediate(const sword value) {
    return (INSTR_SIGN & value) |
           ((BITMASK(11) & value) >> 4) |
           ((BITSMASK(10, 5) & value) << 20) |
           ((BITSMASK(4, 1) & value) << 7);
}

const word utype_immediate(const sword value) {
    return BITSMASK(31, 12) & value;
}

const word jtype_immediate(const sword value) {
    return (INSTR_SIGN & value) |
           (BITSMASK(19, 12) & value) |
           ((BITMASK(11) & value) << 9) |
           ((BITSMASK(10, 1) & value) << 20);
}

// ---------- Test Services ----------

dword z_test_load_dword(dword address) { return 0; }
void z_test_store_dword(dword address, dword value) { return; }

word z_test_load_word(dword address) { return 0; }
void z_test_store_word(dword address, word value) { return; }

hword z_test_load_hword(dword address) { return 0; }
void z_test_store_hword(dword address, hword value) { return; }

byte z_test_load_byte(dword address) { return 0; }
void z_test_store_byte(dword address, byte value) { return; }

void z_test_log_trace(unsigned step, dword pc, dword *registers) { return; }

void z_test_log_message(const char *msg) { fprintf(stdout, msg); }
void z_test_panic(const char *msg)       { fprintf(stderr, msg); }

rsk_host_services_t test_services = {
    .mem_load_dword =  z_test_load_dword,
    .mem_store_dword = z_test_store_dword,
    .mem_load_word =   z_test_load_word,
    .mem_store_word =  z_test_store_word,
    .mem_load_hword =  z_test_load_hword,
    .mem_store_hword = z_test_store_hword,
    .mem_load_byte =   z_test_load_byte,
    .mem_store_byte =  z_test_store_byte,
    .log_trace =       z_test_log_trace,
    .log_msg =         z_test_log_message,
    .panic =           z_test_panic
};

#endif