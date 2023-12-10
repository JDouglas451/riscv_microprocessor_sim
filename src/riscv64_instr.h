#ifndef SIM_RISCV64_INSTR
#define SIM_RISCV64_INSTR

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

// Simulator data structures and typedefs
#include "riscv_config.h"
#include "riscv64_cpu.h"

// ---------- RISC-V Instruction Definitions ----------

// A RISC-V instruction type; used for decoding, disassembly, and execution
typedef struct riscv64_instruction_type {
	// Mask of the required fields for this instruction type
	dword mask;

	// Required bits within the mask for this instruction type
	dword required_bits;

	// Disassemble an instruction of this type, putting the result into the provided string buffer. Returns the length of the complete disassembled instruction. If the buffer is not large enough to hold the instruction, the resulting string will be terminated early.
	size_t (*disassemble)(riscv_cpu_t* const cpu, dword instr, char* buffer, size_t buffer_size);

	// Execute an instruction of this type and return 1 if pc was changed (zero otherwise)
	void (*execute)(riscv_cpu_t* const cpu, dword instr, int* updated_pc);
} riscv_instr_t;

// A set of potentially many lists of instruction types
typedef struct riscv64_instruction_type_registry {
    // The number of instruction types in the registry
    size_t count;

    // An array of pointers to RISC-V 64 bit instruction type structs
    riscv_instr_t** type_links;
} riscv_registry_t;

// ---------- Instruction Definition Functions -----------

// Append a set of instruction types to a registry and return the number of instructions added
size_t registry_append(riscv_registry_t* const registry, size_t new_types_count, riscv_instr_t* const new_types) {
    size_t og_reg_count = registry->count;

    // resize registry
    if (NULL == registry->type_links) {
        registry->type_links = (riscv_instr_t**) malloc(new_types_count * sizeof(riscv_instr_t*));
        if (NULL == registry->type_links) return 0;
    } else {
        riscv_instr_t** new_type_links = (riscv_instr_t**) realloc(registry->type_links, (og_reg_count + new_types_count) * sizeof(riscv_instr_t*));
        
        if (NULL == new_type_links) return 0;

        registry->type_links = new_type_links;
    }

    registry->count = og_reg_count + new_types_count;

    // append type pointers
    for (size_t i = 0; i < new_types_count; i ++) {
        registry->type_links[og_reg_count + i] = &(new_types[i]);
    }

    return new_types_count;
}

// Return the pointer to the first instruction in the registry that matches the instruction (or NULL if no match was found)
riscv_instr_t* registry_search(const riscv_registry_t* const registry, dword instr) {
    for (size_t i = 0; i < registry->count; i++) {
        riscv_instr_t* itype = registry->type_links[i];
        if (itype->mask & instr == itype->required_bits) return itype;
    }

    return NULL;
}

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

// ---------- Instruction Decomposition Functions ----------

// Isolate the opcode of an instruction
static inline byte mask_instr_opcode(dword instruction) { return (byte) (instruction & INSTR_OPCODE); }

// Isolate the rd field of an R, I, U, or J format instruction
static inline byte mask_instr_rd(dword instruction) { return (byte) ((instruction & INSTR_RD) >> 7); }

// Isolate the funct3 field of an R, I, S, or B format instruction
static inline byte mask_instr_funct3(dword instruction) { return (byte) ((instruction & INSTR_FUNCT3) >> 12); }

// Isolate the rs1 field of an R, I, S, or B format instruction
static inline byte mask_instr_rs1(dword instruction) { return (byte) ((instruction & INSTR_RS1) >> 15); }

// Isolate the rs2 field of an R, S, or B format instruction
static inline byte mask_instr_rs2(dword instruction) { return (byte) ((instruction & INSTR_RS2) >> 20); }

// Isolate the funct7 field of an R format instruction
static inline byte mask_instr_funct7(dword instruction) { return (byte) ((instruction & INSTR_FUNCT7) >> 25); }

// ---------- Instruction Immediate Decoding Functions ----------

// Decode an unsigned immediate value from an I type instruction
static inline dword unsigned_itype_imm(dword instr) {
	return (BITSMASK(31, 20) & instr) >> 20;
}

// Decode a signed immediate value from an I type instruction
static inline sdword itype_imm(dword instr) {
	return (((sdword) unsigned_itype_imm(instr)) << 52) >> 52;
}

// Decode an unsigned immediate value from an S type instruction
static inline dword unsigned_stype_imm(dword instr) {
	return ((BITSMASK(31, 25) & instr) >> 20) |
	       ((BITSMASK(11, 7) & instr) >> 7);
}

// Decode a signed immediate value from an S type instruction
static inline sdword stype_imm(dword instr) {
	return (((sdword) unsigned_stype_imm(instr)) << 52) >> 52;
}

// Decode an unsigned immediate value from a B type instruction
static inline dword unsigned_btype_imm(dword instr) {
	return ((INSTR_SIGN & instr) >> 19) |
	       ((BITSMASK(30, 25) & instr) >> 20) |
		   ((BITSMASK(11, 8) & instr) >> 7) |
		   ((BITMASK(7) & instr) << 4);
}

// Decode a signed immediate value from a B type instruction
static inline sdword btype_imm(dword instr) {
	return (((sdword) unsigned_btype_imm(instr)) << 51) >> 51;
}

// Decode an unsigned immediate value from a U type instruction
static inline dword unsigned_utype_imm(dword instr) {
	return BITSMASK(31, 12) & instr;
}

// Decode a signed immediate value from a U type instruction
static inline sdword utype_imm(dword instr) {
	return (sdword) unsigned_utype_imm(instr);
}

// Decode an unsigned immediate value from a J type instruction
static inline dword unsigned_jtype_imm(dword instr) {
	return ((INSTR_SIGN & instr) >> 11) |
	       ((BITSMASK(30, 21) & instr) >> 20) |
		   ((BITMASK(20) & instr) >> 9) |
		   (BITSMASK(19, 12) & instr);
}

// Decode a signed immediate value from a J type instruction
static inline sdword jtype_imm(dword instr) {
	return (((sdword) unsigned_jtype_imm(instr)) << 43) >> 43;
}

// ---------- Disassembly/Execution Function Names ----------

#define DISASM_DEF(name) size_t _disasm_##name(riscv_cpu_t* const cpu, dword instr, char* buffer, size_t buffer_size)
#define EXEC_DEF(name)   void _exec_##name(riscv_cpu_t* const cpu, dword instr, int* updated_pc)

#define DISASM_FMT(format, ...) snprintf(buffer, buffer_size, format __VA_OPT__(, ) __VA_ARGS__)

#define INSTR_LINKS(name) .disassemble = _disasm_##name##, .execute = _exec_##name

#define READ_REG(index)         cpu_read_register(cpu, index)
#define WRITE_REG(index, value) cpu_write_register(cpu, index, value)

#define GET_PC       cpu->pc
#define SET_PC(addr) *updated_pc = 1; cpu->pc = addr

#define GET_FUNCT7 mask_instr_funct7(instr)
#define GET_RS2    mask_instr_rs2(instr)
#define GET_RS1    mask_instr_rs1(instr)
#define GET_FUNCT3 mask_instr_funct3(instr)
#define GET_RD     mask_instr_rd(instr)
#define GET_OPCODE mask_instr_opcode(instr)

#define STORE_BYTE(addr, value)  cpu->host.mem_store_byte(addr, value)
#define STORE_HWORD(addr, value) cpu->host.mem_store_hword(addr, value)
#define STORE_WORD(addr, value)  cpu->host.mem_store_word(addr, value)
#define STORE_DWORD(addr, value) cpu->host.mem_store_dword(addr, value)

#define LOAD_BYTE(addr)  cpu->host.mem_load_byte(addr)
#define LOAD_HWORD(addr) cpu->host.mem_load_hword(addr)
#define LOAD_WORD(addr)  cpu->host.mem_load_word(addr)
#define LOAD_DWORD(addr) cpu->host.mem_load_dword(addr)

#endif