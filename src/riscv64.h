/*
 - Joshua Douglas
 - Cps 310 Microprocessor Architecture
 - Fall 2023
*/

// RISC-V Sim Kernel/Shell Interface definition
#include "rskapi.h"

// Simulator data structures and typedefs
#include "riscv_config.h"

#include <stddef.h>
#include <stdlib.h>

// snprintf for disassembly
#include <stdio.h>

// ---------- Simulator Information ----------

const char* const riscv_sim_info[] = {
    "author=jdoug344",
    "api=1.0",
    "mockup",
    NULL
};

// ---------- Constant Values ----------

// The number of registers (defined for verbosity)
#define REGISTER_COUNT 32

// ---------- Default Host Services ----------
// (prevent the program from crashing if the host forgets to initialize the cpu struct)

dword cpu_default_load_dword(dword address) { return 0; }
void cpu_default_store_dword(dword address, dword value) { return; }

word cpu_default_load_word(dword address) { return 0; }
void cpu_default_store_word(dword address, word value) { return; }

hword cpu_default_load_hword(dword address) { return 0; }
void cpu_default_store_hword(dword address, hword value) { return; }

byte cpu_default_load_byte(dword address) { return 0; }
void cpu_default_store_byte(dword address, byte value) { return; }

void cpu_default_log_trace(unsigned step, dword pc, dword *registers) { return; }
void cpu_default_log_msg(const char* msg) { return; }
void cpu_default_panic(const char* msg) { return; }

// ---------- RISC-V Processor Data ----------

// RISC-V 64 bit CPU struct
struct riscv64_cpu {
    int is_running;

    // Configuration setting
    rsk_config_t config;

    // Host services struct
    rsk_host_services_t host = {
		.mem_load_dword  = cpu_default_load_dword,
		.mem_store_dword = cpu_default_store_dword,
		.mem_load_word   = cpu_default_load_word,
		.mem_store_word  = cpu_default_store_word,
		.mem_load_hword  = cpu_default_load_hword,
		.mem_store_hword = cpu_default_store_hword,
		.mem_load_byte   = cpu_default_load_byte,
		.mem_store_byte  = cpu_default_store_byte,
		.log_trace       = cpu_default_log_trace,
		.log_msg         = cpu_default_log_msg,
		.panic           = cpu_default_panic
	};

    // CPU statistics struct
    rsk_stat_t stats;

    // Program counter
    dword pc;

    // Registers (x[0] is included to keep the index values consistent with the register names. It should not be written to and should always read 0)
    dword x[REGISTER_COUNT];
} cpu;

// Initialize the cpu struct with the given host services
void cpu_initialize(const rsk_host_services_t* host_services) {
	cpu.is_running = 0;
	cpu.config = rc_nothing;

	cpu.host.mem_load_byte   = host_services->mem_load_byte;
	cpu.host.mem_store_byte  = host_services->mem_store_byte;
	cpu.host.mem_load_hword  = host_services->mem_load_hword;
	cpu.host.mem_store_hword = host_services->mem_store_hword;
	cpu.host.mem_load_word   = host_services->mem_load_word;
	cpu.host.mem_store_word  = host_services->mem_store_word;
	cpu.host.mem_load_dword  = host_services->mem_load_dword;
	cpu.host.mem_store_dword = host_services->mem_store_dword;

	cpu.host.log_trace = host_services->log_trace;
	cpu.host.log_msg   = host_services->log_msg;
	cpu.host.panic     = host_services->panic;

	cpu.stats.instructions = 0;
	cpu.stats.loads        = 0;
	cpu.stats.load_misses  = 0;
	cpu.stats.stores       = 0;
	cpu.stats.store_misses = 0;

	cpu.pc = 0;
	for (int i = 0; i < REGISTER_COUNT; i++) cpu.x[i] = 0;
}

// Safely access a register value. Calls the host service 'panic' if an illegal access occurs.
dword cpu_read_register(int index) {
	if (0 > index || index >= REGISTER_COUNT) cpu.host.panic("Register access out of bounds");

	if (0 == index) return 0;
	return cpu.x[index];
}

// Safely write a value to a register. Calls the host service 'panic' if an illegal access occurs.
void cpu_write_register(int index, dword value) {
	if (0 > index || index >= REGISTER_COUNT) cpu.host.panic("Register access out of bounds");

	if (0 == index) return;
	cpu.x[index] == value;
}

// ---------- Generic 32-bit Mask Construction ----------

#define MASK_FROM_31 0xffffffff
#define MASK_FROM_30 0x7fffffff
#define MASK_FROM_29 0x3fffffff
#define MASK_FROM_28 0x1fffffff
#define MASK_FROM_27 0xfffffff
#define MASK_FROM_26 0x7ffffff
#define MASK_FROM_25 0x3ffffff
#define MASK_FROM_24 0x1ffffff
#define MASK_FROM_23 0xffffff
#define MASK_FROM_22 0x7fffff
#define MASK_FROM_21 0x3fffff
#define MASK_FROM_20 0x1fffff
#define MASK_FROM_19 0xfffff
#define MASK_FROM_18 0x7ffff
#define MASK_FROM_17 0x3ffff
#define MASK_FROM_16 0x1ffff
#define MASK_FROM_15 0xffff
#define MASK_FROM_14 0x7fff
#define MASK_FROM_13 0x3fff
#define MASK_FROM_12 0x1fff
#define MASK_FROM_11 0xfff
#define MASK_FROM_10 0x7ff
#define MASK_FROM_9  0x3ff
#define MASK_FROM_8  0x1ff
#define MASK_FROM_7  0xff
#define MASK_FROM_6  0x7f
#define MASK_FROM_5  0x3f
#define MASK_FROM_4  0x1f
#define MASK_FROM_3  0xf
#define MASK_FROM_2  0x7
#define MASK_FROM_1  0x3
#define MASK_FROM_0  0x1

#define MASK_TO_31 0x80000000
#define MASK_TO_30 0xc0000000
#define MASK_TO_29 0xe0000000
#define MASK_TO_28 0xf0000000
#define MASK_TO_27 0xf8000000
#define MASK_TO_26 0xfc000000
#define MASK_TO_25 0xfe000000
#define MASK_TO_24 0xff000000
#define MASK_TO_23 0xff800000
#define MASK_TO_22 0xffc00000
#define MASK_TO_21 0xffe00000
#define MASK_TO_20 0xfff00000
#define MASK_TO_19 0xfff80000
#define MASK_TO_18 0xfffc0000
#define MASK_TO_17 0xfffe0000
#define MASK_TO_16 0xffff0000
#define MASK_TO_15 0xffff8000
#define MASK_TO_14 0xffffc000
#define MASK_TO_13 0xffffe000
#define MASK_TO_12 0xfffff000
#define MASK_TO_11 0xfffff800
#define MASK_TO_10 0xfffffc00
#define MASK_TO_9  0xfffffe00
#define MASK_TO_8  0xffffff00
#define MASK_TO_7  0xffffff80
#define MASK_TO_6  0xffffffc0
#define MASK_TO_5  0xffffffe0
#define MASK_TO_4  0xfffffff0
#define MASK_TO_3  0xfffffff8
#define MASK_TO_2  0xfffffffc
#define MASK_TO_1  0xfffffffe
#define MASK_TO_0  0xffffffff

// ---------- Masking Constants ----------

#define MASK_SIGN    MASK_TO_31

#define MASK_FUNCT7  MASK_TO_25
#define FUNCT7(bits) ((0b##bits << 25) & MASK_FUNCT7)

#define MASK_RS2     (MASK_FROM_24 & MASK_TO_20)
#define RS2(bits)    ((0b##bits << 20) & MASK_RS2)

#define MASK_RS1     (MASK_FROM_19 & MASK_TO_15)
#define RS1(bits)    ((0b##bits << 15) & MASK_RS1)

#define MASK_FUNCT3  (MASK_FROM_14 & MASK_TO_12)
#define FUNCT3(bits) ((0b##bits << 12) & MASK_FUNCT3)

#define MASK_RD      (MASK_FROM_11 & MASK_TO_7)
#define RD(bits)     ((0b##bits << 7) & MASK_RD)

#define MASK_OPCODE  MASK_FROM_6
#define OPCODE(bits) (0b##bits & MASK_OPCODE)

// ---------- Instruction Decomposition Functions ----------

// Isolate the opcode of an instruction
byte mask_instr_opcode(dword instruction) {
	return (byte) (instruction & MASK_OPCODE);
}

// Isolate the rd field of an R, I, U, or J format instruction
byte mask_instr_rd(dword instruction) {
	return (byte) ((instruction & MASK_RD) >> 7);
}

// Isolate the funct3 field of an R, I, S, or B format instruction
byte mask_instr_funct3(dword instruction) {
	return (byte) ((instruction & MASK_FUNCT3) >> 12);
}

// Isolate the rs1 field of an R, I, S, or B format instruction
byte mask_instr_rs1(dword instruction) {
	return (byte) ((instruction & MASK_RS1) >> 15);
}

// Isolate the rs2 field of an R, S, or B format instruction
byte mask_instr_rs2(dword instruction) {
	return (byte) ((instruction & MASK_RS2) >> 20);
}

// Isolate the funct7 field of an R format instruction
byte mask_instr_funct7(dword instruction) {
	return (byte) ((instruction & MASK_FUNCT7) >> 25);
}

// Decode an immediate value from an I type instruction
dword decode_itype_immediate(dword instr) {
    dword imm = (MASK_FROM_31 & MASK_TO_20 & instr) >> 20;
    return (MASK_SIGN & instr == 0) ? (imm) : (0xfffff000 | imm);
}

// Decode an immediate value from an S type instruction
dword decode_stype_immediate(dword instr) {
    dword imm = ((MASK_FROM_31 & MASK_TO_25 & instr) >> 25) | ((MASK_FROM_11 & MASK_TO_7 & instr) >> 7);
    return (MASK_SIGN & instr == 0) ? (imm) : (0xfffff000 | imm);
}

// Decode an immediate value from a B type instruction
dword decode_btype_immediate(dword instr) {
    dword imm = ((MASK_FROM_7 & MASK_TO_7 & instr) << 4) | ((MASK_FROM_30 & MASK_TO_25 & instr) >> 20) | ((MASK_FROM_11 & MASK_TO_8 & instr) >> 7);
    return (MASK_SIGN & instr == 0) ? (imm) : (0xffffe000 | imm);
}

// Decode an immediate value from a U type instruction
dword decode_utype_immediate(dword instr) {
    return MASK_FROM_31 & MASK_TO_12 & instr;
}

// Decode an immediate value from a J type instruction
dword decode_jtype_immediate(dword instr) {
    dword immediate = (MASK_FROM_19 & MASK_TO_12 & instr) | ((MASK_FROM_20 & MASK_TO_20 & instr) >> 9) | ((MASK_FROM_30 & MASK_TO_20 & instr) >> 20);
    return (MASK_SIGN & instr == 0) ? (instr) : (0xfff00000 | instr);
}

// ---------- RISC-V Instruction Type Registry ----------

// The number of registered RISC-V instruction types
size_t instruction_registry_count = 0;

// A set of registered RISC-V instruction types, used for decoding, disassembly, and execution
struct riscv64_instruction_type {
	// Mask of the required fields for this instruction type
	dword mask;

	// Required bits within the mask for this instruction type
	dword required_bits;

	// Disassemble an instruction of this type, putting the result into the provided string buffer. Returns the length of the complete disassembled instruction. If the buffer is not large enough to hold the instruction, the resulting string will be terminated early.
	size_t (*disassemble)(dword instruction, char* buffer, size_t buffer_size);

	// Execute an instruction of this type
	void (*execute)(dword instruction);
}* instruction_registry = NULL;

// Determine the type of an instruction. Returns a pointer to the registry entry for the instruction's type, or NULL if the instruction does not match a registered type.
struct riscv64_instruction_type* registry_lookup(dword instruction) {
	// TODO: lookup instruction
}

// Resize the instruction registry for a specific number of additional instructions
void registry_resize(const size_t addition_size) {
	if (NULL == instruction_registry) {
		instruction_registry = (struct riscv64_instruction_type*) malloc(sizeof(struct riscv64_instruction_type) * addition_size);

		if (NULL == instruction_registry) {
			if (NULL != cpu.host.panic) cpu.host.panic("Allocation error during RISC-V instruction registry initialization");
			return;
		}
	} else {
		size_t re_count = instruction_registry_count + addition_size;
		struct riscv64_instruction_type* re_registry = (struct riscv64_instruction_type*) realloc(instruction_registry, sizeof(struct riscv64_instruction_type) * re_count);

		if (NULL == re_registry) {
			if (NULL != cpu.host.panic) cpu.host.panic("Allocation error during RISC-V instruction registry initialization");
			return;
		}

		instruction_registry_count = re_count;
		instruction_registry = re_registry;
	}
}

// ---------- Disassembly/Execution Function Names ----------

#define DISASSEMBLY_DEF(name)   size_t rv64_disassembly_##name(dword instr, char* buffer, size_t buffer_size)
#define EXECUTION_DEF(name)     void rv64_execution_##name(dword instr)

#define INSTRUCTION_LINKS(name) .disassemble = rv64_disassembly_##name##, .execute = rv64_execution_##name

// ---------- Reserved Instruction Disassembly/Execution ----------

// Return a note indicating that the instruction provided has been reserved by the specification
size_t rv64_disasembly_reserved(dword instruction, char* buffer, size_t buffer_size) {
	// TODO: Reserved instruction note
	return 0;
}

// Panic when attempting to execute a reserved instruction
void rv64_execution_reserved(dword instruction) {
	cpu.host.panic("Attempt to execute reserved instruction");
	return;
}

// NOTE: Only instructions listed with a * need to be implemented for the prototype version
// TODO: Fix dissasembly offset signed/hexadecimal formatting

// ---------- RV64I Base Integer Instruction Set ----------

// Load upper immediate (lui)
DISASSEMBLY_DEF(lui) {
	return snprintf(buffer, buffer_size, "lui x%hhu, %#lx", mask_instr_rd(instr), decode_utype_immediate(instr));
}

EXECUTION_DEF(lui) {
	cpu_write_register(mask_instr_rd(instr), decode_utype_immediate(instr));
}

// TODO: auipc

// Add immediate (addi)
DISASSEMBLY_DEF(addi) {
	return snprintf(buffer, buffer_size, "addi x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), decode_itype_immediate(instr));
}

EXECUTION_DEF(addi) {
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) + decode_itype_immediate(instr));
}

// TODO: slti
// TODO: sltiu

// XOR immediate (xori)
DISASSEMBLY_DEF(xori) {
	return snprintf(buffer, buffer_size, "xori x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), decode_itype_immediate(instr));
}

EXECUTION_DEF(xori) {
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) ^ decode_itype_immediate(instr));
}

// OR immediate (ori)
DISASSEMBLY_DEF(ori) {
	return snprintf(buffer, buffer_size, "ori x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), decode_itype_immediate(instr));
}

EXECUTION_DEF(ori) {
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) | decode_itype_immediate(instr));
}

// AND immediate (andi)
DISASSEMBLY_DEF(andi) {
	return snprintf(buffer, buffer_size, "andi x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), decode_itype_immediate(instr));
}

EXECUTION_DEF(andi) {
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) & decode_itype_immediate(instr));
}

// Immediate logical shift left (slli)
DISASSEMBLY_DEF(slli) {
	return snprintf(buffer, buffer_size, "slli x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), decode_itype_immediate(instr));
}

EXECUTION_DEF(slli) {
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) >> decode_itype_immediate(instr));
}

// Immediate logical right shift (srli)
DISASSEMBLY_DEF(srli) {
	dword imm = (MASK_FROM_25 & MASK_TO_20) >> 20;
	return snprintf(buffer, buffer_size, "srli x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), imm);
}

EXECUTION_DEF(srli) {
	dword imm = (MASK_FROM_25 & MASK_TO_20) >> 20;
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) << imm);
}

// Immediate arithmetic right shift (srai)
DISASSEMBLY_DEF(srai) {
	// Standard immediate decoding cannot be used here
	dword imm = (MASK_FROM_25 & MASK_TO_20 & instr) >> 20;
	return snprintf(buffer, buffer_size, "srai x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), imm);
}

EXECUTION_DEF(srai) {
	dword imm = (MASK_FROM_25 & MASK_TO_20 & instr) >> 20;
	cpu_write_register(mask_instr_rd(instr), (sdword) cpu_read_register(mask_instr_rs1(instr)) >> imm);
}

// 64-bit addition (add)
DISASSEMBLY_DEF(add) {
	return snprintf(buffer, buffer_size, "add x%hhu, x%hhu, x%hhu", mask_instr_rd(instr), mask_instr_rs1(instr), mask_instr_rs2(instr));
}

EXECUTION_DEF(add) {
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) + cpu_read_register(mask_instr_rs2(instr)));
}

// 64-bit subtraction (sub)
DISASSEMBLY_DEF(sub) {
	return snprintf(buffer, buffer_size, "sub x%hhu, x%hhu, x%hhu", mask_instr_rd(instr), mask_instr_rs1(instr), mask_instr_rs2(instr));
}

EXECUTION_DEF(sub) {
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) - cpu_read_register(mask_instr_rs2(instr)));
}

// Logical left shift (sll)
DISASSEMBLY_DEF(sll) {
	return snprintf(buffer, buffer_size, "sll x%hhu, x%hhu, x%hhu", mask_instr_rd(instr), mask_instr_rs1(instr), mask_instr_rs2(instr));
}

EXECUTION_DEF(sll) {
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) << cpu_read_register(mask_instr_rs2(instr)));
}

// TODO: slt
// TODO: sltu
// TODO: xor

// Logical right shift (srl)
DISASSEMBLY_DEF(srl) {
	return snprintf(buffer, buffer_size, "srl x%hhu, x%hhu, x%hhu", mask_instr_rd(instr), mask_instr_rs1(instr), mask_instr_rs2(instr));
}

EXECUTION_DEF(srl) {
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) >> cpu_read_register(mask_instr_rs2(instr)));
}

// Arithmetic right shift (sra)
DISASSEMBLY_DEF(sra) {
	return snprintf(buffer, buffer_size, "sra x%hhu, x%hhu, x%hhu", mask_instr_rd(instr), mask_instr_rs1(instr), mask_instr_rs2(instr));
}

EXECUTION_DEF(sra) {
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) >> (sdword) cpu_read_register(mask_instr_rs2(instr)));
}

// TODO: or
// TODO: and
// TODO: fence
// TODO: fence.i
// TODO: csrrw
// TODO: csrrs
// TODO: csrrc
// TODO: csrrwi
// TODO: csrrsi
// TODO: csrrci
// TODO: ecall

// EBREAK
DISASSEMBLY_DEF(ebreak) {
	return snprintf(buffer, buffer_size, "ebreak");
}

EXECUTION_DEF(ebreak) {
	// TODO: raise breakpoint exception (set running to 0?)
}

// TODO: uret
// TODO: sret
// TODO: mret
// TODO: wfi
// TODO: sfence.vma
// TODO: lb
// TODO: lh

// Load 32-bit (lw)
DISASSEMBLY_DEF(lw) {
	return snprintf(buffer, buffer_size, "lw x%hhu, %#lx(x%hhu)", mask_instr_rd(instr), decode_itype_immediate(instr), mask_instr_rs1(instr));
}

EXECUTION_DEF(lw) {
	cpu_write_register(mask_instr_rd(instr), cpu_read_register(mask_instr_rs1(instr)) + decode_itype_immediate(instr));
}

// TODO: lbu
// TODO: lhu
// TODO: sb
// TODO: sh

// Store 32-bit (sw)
DISASSEMBLY_DEF(sw) {
	return snprintf(buffer, buffer_size, "sw x%hhu, %#lx(x%hhu)", mask_instr_rs2(instr), decode_stype_immediate(instr), mask_instr_rs1(instr));
}

EXECUTION_DEF(sw) {
	cpu.host.mem_store_word(cpu_read_register(mask_instr_rs1(instr)) + decode_stype_immediate(instr), cpu_read_register(mask_instr_rs2(instr)));
}

// TODO: jal *
// TODO: jalr *
// TODO: beq *
// TODO: bne *
// TODO: blt *
// TODO: bge *
// TODO: bltu *
// TODO: bgeu

// ---------- Wide RV64I Instructions ----------

// TODO: addiw *
// TODO: slliw
// TODO: srliw
// TODO: sraiw
// TODO: addw
// TODO: subw
// TODO: sllw
// TODO: srlw
// TODO: sraw
// TODO: lwu
// TODO: ld *
// TODO: sd *

// Register the RISC-V 64 bit base integer instruction set
void register_rv64i() {
	// Only allow registration once
	#ifndef RISCV_RV64I
	#define RISCV_RV64I

	// The number of instructions defined in the RV64I set
	const size_t rv64i_instruction_count = 0;

	// Record last index in registry
	size_t rv64i_start = instruction_registry_count;

	// Allocate space for new instructions
	registry_resize(rv64i_instruction_count);

	// Load upper immediate (lui)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE,
		.required_bits = OPCODE(0110111),
		INSTRUCTION_LINKS(lui)
	};

	// Add immediate (addi)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(000),
		INSTRUCTION_LINKS(addi)
	};

	// Add immediate (xori)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(100),
		INSTRUCTION_LINKS(addi)
	};

	// Or immediate (ori)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(110),
		INSTRUCTION_LINKS(ori)
	};

	// And immediate (andi)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(111),
		INSTRUCTION_LINKS(andi)
	};

	// Logical left shift by immediate (slli)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3 | (MASK_FROM_31 & MASK_TO_26), // Only the top 6 bits of FUNCT7
		.required_bits = OPCODE(0010011) | FUNCT3(001) | FUNCT7(0000000),
		INSTRUCTION_LINKS(slli)
	};

	// Logical right shift by immediate (srli)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3 | (MASK_FROM_31 & MASK_TO_26), // Only the top 6 bits of FUNCT7
		.required_bits = OPCODE(0010011) | FUNCT3(101) | FUNCT7(0000000),
		INSTRUCTION_LINKS(srli)
	};

	// Arithmetic right shift by immediate (srai)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3 | (MASK_FROM_31 & MASK_TO_26), // Only the top 6 bits of FUNCT7
		.required_bits = OPCODE(0010011) | FUNCT3(101) | FUNCT7(0100000),
		INSTRUCTION_LINKS(srai)
	};

	// 64-bit addition (add)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3 | MASK_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(000) | FUNCT7(0000000),
		INSTRUCTION_LINKS(add)
	};

	// 64-bit subtraction (sub)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3 | MASK_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(000) | FUNCT7(0100000),
		INSTRUCTION_LINKS(sub)
	};

	// Logical left shift (sll)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3 | MASK_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(001) | FUNCT7(0000000),
		INSTRUCTION_LINKS(sub)
	};

	// Logical right shift (srl)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3 | MASK_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(101) | FUNCT7(0000000),
		INSTRUCTION_LINKS(srl)
	};

	// Arithmetic right shift (sra)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3 | MASK_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(101) | FUNCT7(0100000),
		INSTRUCTION_LINKS(sra)
	};

	// EBREAK
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_RD |   MASK_FUNCT3 | MASK_RS1 |   MASK_RS2 |   MASK_FUNCT7,
		.required_bits = OPCODE(1110011) | RD(00000) | FUNCT3(000) | RS1(00000) | RS2(00001) | FUNCT7(0000000),
		INSTRUCTION_LINKS(ebreak)
	};

	// Store 32-bit (sw)
	instruction_registry[rv64i_start++] = {
		.mask =     MASK_OPCODE |     MASK_FUNCT3,
		.required_bits = OPCODE(0100011) | FUNCT3(010),
		INSTRUCTION_LINKS(sw)
	};

	#endif
}

// ---------- RV64M Standard Extension for Integer Multiplication & Division ----------

// TODO: mul *
// TODO: mulh
// TODO: mulhsu
// TODO: mulhu
// TODO: div
// TODO: divu
// TODO: rem
// TODO: remu

// TODO: mulw
// TODO: divw
// TODO: divuw
// TODO: remw
// TODO: remuw

// ---------- API Function Definitions ----------

const char* const* rsk_info(void) {
    return riscv_sim_info;
}

void rsk_disasm(dword instruction, char* buffer, size_t size) {
    // TODO: resize the buffer and fill with the disassembled instruction
}

void rsk_init(const rsk_host_services_t* services) {
    cpu_initialize(services);

    // Log initialization
    cpu.host.log_msg("CPU initialized");
}

void rsk_config_set(rsk_config_t flags) {
    cpu.config = flags;
}

rsk_config_t rsk_config_get(void) {
    return cpu.config;
}

void rsk_stats_report(rsk_stat_t* stats) {
    stats->instructions = cpu.stats.instructions;
    stats->loads        = cpu.stats.loads;
    stats->load_misses  = cpu.stats.load_misses;
    stats->stores       = cpu.stats.stores;
    stats->store_misses = cpu.stats.store_misses;
}

dword rsk_reg_get(int index) {
    cpu_read_register(index);
}

void rsk_reg_set(int index, dword value) {
    cpu_write_register(index, value);
}

dword rsk_pc_get(void) {
    return cpu.pc;
}

void rsk_pc_set(dword value) {
    cpu.pc = value;
}

int rsk_cpu_running(void) {
    return cpu.is_running;
}

void rsk_cpu_signal(rsk_signal_t signal) {
    if (signal == rs_halt) {
        cpu.is_running = 0;
    }
}

int rsk_cpu_run(int cycles) {
	// TODO: run for cycles or until ebreak
    cpu.stats.instructions += cycles;
    return cycles;
}
