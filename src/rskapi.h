/*
 - RISC-V Sim Kernel API
 - Defines interface between RISC-V Sim kernels (CPUs) and hosts (systems)
 - (c) 2016-2023, Bob Jones University
*/

#ifndef RISCV_SIM_KERNEL_API
#define RISCV_SIM_KERNEL_API

#include <stddef.h>
#include <stdint.h>

// Fundamental unit typedefs
typedef uint8_t byte;
typedef int8_t sbyte;
typedef uint16_t hword;
typedef int16_t shword;
typedef uint32_t word;
typedef int32_t sword;
typedef uint64_t dword;
typedef int64_t sdword;

// rskAPI functions must be exported with "C" linkage, even if implemented in C++
#ifdef __cplusplus
extern "C" {
#endif

// What sort of signal are we giving the CPU? (used to control concurrent simulation on a background thread)
typedef enum rsk_signal {
	rs_halt,
} rsk_signal_t;

/* Configuration bit flags */
typedef enum rsk_config {
	// Default setting, no special features enabled
	rc_nothing = 0x00000000,
	// Require a trace log after every instruction
	rc_trace_log = 0x00000001,
} rsk_config_t;

// Structure of function pointers for services provided by the host
typedef struct rsk_host_services {
	// Load a dword from memory (or MMIO) at address
	dword (*mem_load_dword)(dword address);

	// Store a dword to memory (or MMIO) at address
	void (*mem_store_dword)(dword address, dword value);

	// Load a word from memory (or MMIO) at address
	word (*mem_load_word)(dword address);

	// Store a word to memory (or MMIO) at address
	void (*mem_store_word)(dword address, word value);

	// Load a halfword from memory (or MMIO) at address
	hword (*mem_load_hword)(dword address);

	// Store a halfword to memory (or MMIO) at address
	void (*mem_store_hword)(dword address, hword value);

	// Load a byte from memory (or MMIO) at address
	byte (*mem_load_byte)(dword address);

	// Store a byte to memory (or MMIO) at address
	void (*mem_store_byte)(dword address, byte value);

	// Signal the host that an instruction was just fetched from the address <pc> and executed
	void (*log_trace)(unsigned step, dword pc, dword *registers);

	// Log a debugging/informational message to the debug log
	void (*log_msg)(const char *msg);

	// Log a fatal error message and terminate simulation
	void (*panic)(const char *msg);
} rsk_host_services_t;

// Structure of event counters maintained/published by the kernel
typedef struct rsk_stats {
	// Number of instructions executed so far
	unsigned int instructions;
	// Number of memory loads (including instruction fetches)
	unsigned int loads;
	// Number of memory stores
	unsigned int stores;
	// Number of loads that were cache misses (for cache-implementing kernels)
	unsigned int load_misses;
	// Number of stores that were cache misses (for cache-implementing kernels)
	unsigned int store_misses;
} rsk_stat_t;

// ----------------- Kernel API ---------------- //
// ----- (functions available to the host) ----- //

// Return a pointer to a NULL-terminated list of NUL-terminated C-strings describing the kernel's features
char** rsk_info(void);

// Disassemble a given 64-bit RISC-V <instruction> into the string buffer provided. The <address> where the instruction is found in memory is only required to calculate branch targets.
void rsk_disasm(dword address, dword instruction, char* buffer, size_t size);

// Reset/initialize the simulated CPU, binding it to the environment provided by the host services functions
void rsk_init(const rsk_host_services_t* services);

// Set the current configuration flags
void rsk_config_set(rsk_config_t flags);

// Get the current configuration flags
rsk_config_t rsk_config_get(void);

// Populate a stats-counter struct with the current CPU performance statistics
void rsk_stats_report(rsk_stat_t* stats);

// Get the value of the indicated register (the only legal register indices are [0-31] inclusive)
dword rsk_reg_get(int index);

// Set the value of the indicated register (the only legal register indices are [0-31] inclusive)
void rsk_reg_set(int index, dword value);

// Get the value of the program counter
dword rsk_pc_get(void);

// Set the value of the program counter
void rsk_pc_set(dword value);

// Returns a nonzero value if the simulated CPU is running
int rsk_cpu_running(void);

// Signal a running CPU of an external event (i.e. a forced halt)
void rsk_cpu_signal(rsk_signal_t signal);

// Start the CPU running for <cycles> instructions, or until EBREAK if <cycles> is 0. Returns the number of instructions executed, which should match <cycles> unless <cycles> was 0.
int rsk_cpu_run(int cycles);

#ifdef __cplusplus
}
#endif

#endif