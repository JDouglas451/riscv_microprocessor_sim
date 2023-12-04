#ifndef RISCV_SIM_CPU
#define RISCV_SIM_CPU

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

// Simulator data structures and typedefs
#include "riscv_config.h"

// ---------- CPU Constants ----------

#define REGISTER_COUNT 32
#define XLEN 64



// ---------- Default Host Services ----------
// (prevent the program from crashing if the host forgets to initialize the cpu struct)

dword default_cpu_load_dword(dword address) { return 0; }
void default_cpu_store_dword(dword address, dword value) { return; }

word default_cpu_load_word(dword address) { return 0; }
void default_cpu_store_word(dword address, word value) { return; }

hword default_cpu_load_hword(dword address) { return 0; }
void default_cpu_store_hword(dword address, hword value) { return; }

byte default_cpu_load_byte(dword address) { return 0; }
void default_cpu_store_byte(dword address, byte value) { return; }

void default_cpu_log_trace(unsigned step, dword pc, dword *registers) { return; }
void default_cpu_log_msg(const char* msg) { return; }
void default_cpu_panic(const char* msg) { return; }

// ---------- CPU Data Structures ----------

// RISC-V bit CPU struct
typedef struct riscv_cpu {
    int is_running;

    // Configuration setting
    rsk_config_t config;

    // Host services struct
    rsk_host_services_t host;
    
    /*
    {
		.mem_load_dword  = default_cpu_load_dword,
		.mem_store_dword = default_cpu_store_dword,
		.mem_load_word   = default_cpu_load_word,
		.mem_store_word  = default_cpu_store_word,
		.mem_load_hword  = default_cpu_load_hword,
		.mem_store_hword = default_cpu_store_hword,
		.mem_load_byte   = default_cpu_load_byte,
		.mem_store_byte  = default_cpu_store_byte,
		.log_trace       = default_cpu_log_trace,
		.log_msg         = default_cpu_log_msg,
		.panic           = default_cpu_panic
	};
    */

    // CPU statistics struct
    rsk_stat_t stats;

    // Program counter
    dword pc;

    // Registers (x[0] is included to keep the index values consistent with the register names. It should not be written to and should always read 0)
    dword x[REGISTER_COUNT];
} riscv_cpu_t;

// ---------- CPU Utility Methods ----------

void init_cpu(const rsk_host_services_t* const services, riscv_cpu_t* cpu) {
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

	cpu->pc = 0;
	for (int i = 0; i < REGISTER_COUNT; i++) cpu->x[i] = 0;
}



#endif