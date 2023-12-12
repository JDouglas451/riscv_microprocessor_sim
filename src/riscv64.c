#include "riscv64.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

// ---------- RISC-V Instruction Definitions ----------

// A RISC-V instruction type; used for decoding, disassembly, and execution
typedef struct riscv64_instruction_type {
    // The name of this instruction type
    const char* const name;

	// Mask of the required fields for this instruction type
	const dword mask;

	// Required bits within the mask for this instruction type
	const dword required_bits;

	// Disassemble an instruction of this type, putting the result into the provided string buffer. Returns the length of the complete disassembled instruction. If the buffer is not large enough to hold the instruction, the resulting string will be terminated early.
	size_t (*disassemble)(riscv_cpu_t* const cpu, dword instr, char* buffer, size_t buffer_size);

	// Execute an instruction of this type and return 1 if pc was changed (zero otherwise)
	void (*execute)(riscv_cpu_t* const cpu, dword instr, int* updated_pc);
} riscv_instr_t;

// A set of potentially many lists of instruction types
typedef struct riscv64_instruction_type_registry {
    // The number of instruction types in the registry
    size_t count;

    // TODO: redesign so that instructions can be added individually with automatically resizing array
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
        if ((itype->mask & instr) == itype->required_bits) return itype;
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

#define DISASM_DEF(name) size_t z_disasm_##name(riscv_cpu_t* const cpu, dword instr, char* buffer, size_t buffer_size)
#define EXEC_DEF(name)   void z_exec_##name(riscv_cpu_t* const cpu, dword instr, int* updated_pc)

#define DISASM_FMT(format, ...) snprintf(buffer, buffer_size, format __VA_OPT__(, ) __VA_ARGS__)

#define INSTR_LINKS(name) .disassemble = z_disasm_##name , .execute = z_exec_##name

#define GET_FUNCT7 mask_instr_funct7(instr)
#define GET_RS2    mask_instr_rs2(instr)
#define GET_RS1    mask_instr_rs1(instr)
#define GET_FUNCT3 mask_instr_funct3(instr)
#define GET_RD     mask_instr_rd(instr)
#define GET_OPCODE mask_instr_opcode(instr)

#define READ_REG(index)         cpu_read_register(cpu, index)
#define WRITE_REG(index, value) cpu_write_register(cpu, index, value)

#define GET_PC       cpu_get_pc(cpu)
#define SET_PC(addr) *updated_pc = 1; cpu_set_pc(cpu, addr)

#define STORE_BYTE(addr, value)  cpu_store_byte(cpu, addr, value)
#define STORE_HWORD(addr, value) cpu_store_hword(cpu, addr, value)
#define STORE_WORD(addr, value)  cpu_store_word(cpu, addr, value)
#define STORE_DWORD(addr, value) cpu_store_dword(cpu, addr, value)

#define LOAD_BYTE(addr)  cpu_load_byte(cpu, addr)
#define LOAD_HWORD(addr) cpu_load_hword(cpu, addr)
#define LOAD_WORD(addr)  cpu_load_word(cpu, addr)
#define LOAD_DWORD(addr) cpu_load_dword(cpu, addr)

// ---------- Instruction Set Headers ----------

#include "rv64i_instr.h"
extern const size_t rv64i_size;
extern riscv_instr_t rv64i_instructions[];

#include "rv64m_instr.h"
extern const size_t rv64m_size;
extern riscv_instr_t rv64m_instructions[];

// ---------- CPU Constants (for verbosity) ----------

#define REGISTER_COUNT 32

// ---------- CPU Data Structures ----------

// RISC-V bit CPU struct
typedef struct riscv64_cpu {
    int is_running;

    // Configuration setting
    rsk_config_t config;

    // Host services struct
    rsk_host_services_t host;
    
    // CPU statistics struct
    rsk_stat_t stats;

	// A registry of implemented risc-v instruction types
	riscv_registry_t instruction_set;

    // Program counter
    dword pc;

    // Registers (x[0] is included to keep the index values consistent with the register names. It should not be written to and should always read 0)
    dword x[REGISTER_COUNT];
} riscv_cpu_t;

// ---------- CPU Methods ----------

riscv_cpu_t* cpu_init(riscv_cpu_t* cpu, const rsk_host_services_t* const services) {
    if (NULL == cpu) {
        // TODO: where should this be freed?
        cpu = (riscv_cpu_t*) calloc(1, sizeof(riscv_cpu_t));

        // malloc failure
        if (NULL == cpu) {
            services->panic("Malloc failure during CPU initialization");
            return NULL;
        }
    }

    cpu->is_running = 0;
	cpu->config = rc_nothing;

	cpu->host.mem_load_byte   = services->mem_load_byte;
	cpu->host.mem_store_byte  = services->mem_store_byte;
	cpu->host.mem_load_hword  = services->mem_load_hword;
	cpu->host.mem_store_hword = services->mem_store_hword;
	cpu->host.mem_load_word   = services->mem_load_word;
	cpu->host.mem_store_word  = services->mem_store_word;
	cpu->host.mem_load_dword  = services->mem_load_dword;
	cpu->host.mem_store_dword = services->mem_store_dword;

	cpu->host.log_trace = services->log_trace;
	cpu->host.log_msg   = services->log_msg;
	cpu->host.panic     = services->panic;

	cpu->stats.instructions = 0;
	cpu->stats.loads        = 0;
	cpu->stats.load_misses  = 0;
	cpu->stats.stores       = 0;
	cpu->stats.store_misses = 0;

	// rv64i
	registry_append(&cpu->instruction_set, rv64i_size, rv64i_instructions);
	// rv64m
	registry_append(&cpu->instruction_set, rv64m_size, rv64m_instructions);

	cpu->pc = 0;
	for (int i = 0; i < REGISTER_COUNT; i++) cpu->x[i] = 0;

    return cpu;
}

int cpu_is_running(const riscv_cpu_t* const cpu) {
    if (NULL == cpu) return 0;
    return cpu->is_running;
}

rsk_config_t cpu_get_config(const riscv_cpu_t* const cpu) {
    if (NULL == cpu) return rc_nothing;
    return cpu->config;
}

void cpu_set_config(riscv_cpu_t* const cpu, rsk_config_t config) {
    if (NULL == cpu) return;
    cpu->config = config;
}

byte cpu_load_byte(const riscv_cpu_t* const cpu, dword address) {
    if (NULL == cpu) return 0;
    return cpu->host.mem_load_byte(address);
}

void cpu_store_byte(const riscv_cpu_t* const cpu, dword address, byte value) {
    if (NULL == cpu) return;
    cpu->host.mem_store_byte(address, value);
}

hword cpu_load_hword(const riscv_cpu_t* const cpu, dword address) {
    if (NULL == cpu) return 0;
    return cpu->host.mem_load_hword(address);
}

void cpu_store_hword(const riscv_cpu_t* const cpu, dword address, hword value) {
    if (NULL == cpu) return;
    cpu->host.mem_store_hword(address, value);
}

word cpu_load_word(const riscv_cpu_t* const cpu, dword address) {
    if (NULL == cpu) return 0;
    return cpu->host.mem_load_word(address);
}

void cpu_store_word(const riscv_cpu_t* const cpu, dword address, word value) {
    if (NULL == cpu) return;
    cpu->host.mem_store_word(address, value);
}

dword cpu_load_dword(const riscv_cpu_t* const cpu, dword address) {
    if (NULL == cpu) return 0;
    return cpu->host.mem_load_dword(address);
}

void cpu_store_dword(const riscv_cpu_t* const cpu, dword address, dword value) {
    if (NULL == cpu) return;
    cpu->host.mem_store_dword(address, value);
}

dword cpu_get_pc(const riscv_cpu_t* const cpu) {
    if (NULL == cpu) return 0;
    return cpu->pc;
}

void cpu_set_pc(riscv_cpu_t* const cpu, dword address) {
    if (NULL == cpu) return;
    cpu->pc = address;
}

dword cpu_read_register(const riscv_cpu_t* const cpu, byte index) {
    if (NULL == cpu) return 0;

	if (0 > index || index >= REGISTER_COUNT) {
        cpu->host.panic("Register access out of bounds");
        return 0;
    }

	if (0 == index) return 0;
	return cpu->x[index];
}

void cpu_write_register(riscv_cpu_t* const cpu, byte index, dword value) {
    if (NULL == cpu) return;

	if (0 > index || index >= REGISTER_COUNT) {
        cpu->host.panic("Register access out of bounds");
        return;
    }

	if (0 == index) return;
	cpu->x[index] = value;
}

void cpu_process_signal(riscv_cpu_t* const cpu, rsk_signal_t signal) {
    if (NULL == cpu) return;

    if (signal == rs_halt) {
        cpu->is_running = 0;
    }
}

void cpu_log_trace(riscv_cpu_t* const cpu) {
    if (NULL == cpu) return;
    cpu->host.log_trace(cpu->stats.instructions, cpu->pc, cpu->x);
}

void cpu_log_message(const riscv_cpu_t* cpu, const char* const message) {
    if (NULL == cpu) return;
    cpu->host.log_msg(message);
}

void cpu_panic(const riscv_cpu_t* const cpu, const char* message) {
    if (NULL == cpu) return;
    cpu->host.panic(message);
}

void cpu_fill_stats(const riscv_cpu_t* const cpu, rsk_stat_t* stats) {
    if (NULL == cpu || NULL == stats) return;

    stats->instructions = cpu->stats.instructions;
    stats->loads = cpu->stats.loads;
    stats->stores = cpu->stats.stores;
    stats->load_misses = cpu->stats.load_misses;
    stats->store_misses = cpu->stats.store_misses;
}

const char* const cpu_identify_instr(riscv_cpu_t* const cpu, dword instr) {
    riscv_instr_t* itype = registry_search(&cpu->instruction_set, instr);
    if (NULL == itype) return NULL;
    return itype->name;
}

void cpu_disassemble_instr(riscv_cpu_t* const cpu, char* buffer, size_t buffer_size, dword instr) {
    if (NULL == cpu) return;

	// TODO: fix extreme and potentially inaccurate solution to buffer validation
	if (buffer_size < 32) return;

	char* bp = buffer;
	size_t bps = buffer_size;
	snprintf(bp, bps, "%#.8lx   ", instr);
	bp += 13;
	bps -= 13;

	// get the instruction type
	riscv_instr_t* itype = registry_search(&cpu->instruction_set, instr);
	if (NULL == itype) {
		bp[0] = '?';
		bp[1] = '\0';
	}

	// disassemble instruction
	itype->disassemble(cpu, instr, bp, bps);
}

void cpu_disassemble(riscv_cpu_t* const cpu, char* buffer, size_t buffer_size) {
    if (NULL == cpu) return;

    dword instr = cpu->host.mem_load_dword(cpu->pc);
    cpu_disassemble_instr(cpu, buffer, buffer_size, instr);
}

int cpu_execute(riscv_cpu_t* const cpu) {
    if (NULL == cpu) return 0;
	cpu->is_running = 1;

	// get current instruction an add to disasm buffer
	dword instr = cpu->host.mem_load_dword(cpu->pc);
	if (instr == RV64I_EBREAK) {
		cpu->is_running = 0;
		return 0;
	}

	// get the instruction type
	riscv_instr_t* itype = registry_search(&cpu->instruction_set, instr);
	if (NULL == itype) {
		cpu->host.panic("Unrecognized instruction!");
        cpu->is_running = 0;
		return 0;
	}

	int updated_pc = 0;
	itype->execute(cpu, instr, &updated_pc);
	if (!updated_pc) cpu->pc += 4;

	return 1;
}


