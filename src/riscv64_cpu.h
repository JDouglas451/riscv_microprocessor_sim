#ifndef SIM_RISCV64_CPU
#define SIM_RISCV64_CPU

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

// Simulator data structures and typedefs
#include "riscv_config.h"
#include "riscv64_instr.h"

// rv64im
#include "rv64i_instr.h"
#include "rv64m_instr.h"

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

// Initialize the cpu with default values and the provided host services
void cpu_init(riscv_cpu_t* cpu, const rsk_host_services_t* const services) {
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
}

// Safely access a register value. Calls the host service 'panic' if an illegal access occurs.
dword cpu_read_register(const riscv_cpu_t* const cpu, int index) {
	if (0 > index || index >= REGISTER_COUNT) cpu->host.panic("Register access out of bounds");

	if (0 == index) return 0;
	return cpu->x[index];
}

// Safely write a value to a register. Calls the host service 'panic' if an illegal access occurs.
void cpu_write_register(const riscv_cpu_t* const cpu, int index, dword value) {
	if (0 > index || index >= REGISTER_COUNT) cpu->host.panic("Register access out of bounds");

	if (0 == index) return;
	cpu->x[index] == value;
}

// Disassemble the current instruction
void cpu_disassemble(const riscv_cpu_t* const cpu, char* buffer, size_t buffer_size) {
	// TODO: fix extreme and potentially inaccurate solution to buffer validation
	if (buffer_size < 32) return;

	char* bp = buffer;
	size_t bps = buffer_size;

	// get current instruction an add to disasm buffer
	dword instr = cpu->host.mem_load_dword(cpu->pc);
	int len = snprintf(bp, bps, "%#.8lx   ");
	bp += 13;
	bps -= 13;

	// get the instruction type
	riscv_instr_t* itype = registry_search(&cpu->instruction_set, instr);
	if (NULL == itype) {
		bp[0] = '?';
		bp[1] = '\0';
	}

	// disassemble instruction
	len = itype->disassemble(cpu, instr, bp, bps);
}

// Execute the instruction at pc and return 0 if ebreak was hit
int cpu_execute(riscv_cpu_t* const cpu) {
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
		return;
	}

	int updated_pc = 0;
	itype->execute(cpu, instr, &updated_pc);
	if (!updated_pc) cpu->pc += 4;

	return 1;
}

#endif