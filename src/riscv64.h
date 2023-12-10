#ifndef SIM_RISCV64
#define SIM_RISCV64

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------- Fundamental unit alias typedefs ----------

// An 8 bit unsigned value
typedef uint8_t byte;

// An 8 bit signed value
typedef int8_t sbyte;

// A 16 bit unsigned value
typedef uint16_t hword;

// A 16 bit signed value
typedef int16_t shword;

// A 32 bit unsigned value
typedef uint32_t word;

// A 32 bit signed value
typedef int32_t sword;

// A 64 bit unsigned value
typedef uint64_t dword;

// A 64 bit signed value
typedef int64_t sdword;

// What sort of signal are we giving the CPU? (used to control concurrent simulation on a background thread)
typedef enum rsk_signal {
	rs_halt,
} rsk_signal_t;

// Configuration bit flags
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

#ifdef __cplusplus
}
#endif

// ---------- CPU Data Structures ----------

// RISC-V bit CPU struct
typedef struct riscv64_cpu riscv_cpu_t;

// ---------- CPU Methods ----------

// Initialize the CPU with default values and the provided host services
void cpu_init(riscv_cpu_t* cpu, const rsk_host_services_t* const services);

// Return 1 if the CPU is running, 0 otherwise
int cpu_is_running(const riscv_cpu_t* const cpu);

// Get the CPU's config setting
rsk_config_t cpu_get_config(const riscv_cpu_t* const cpu);

// Set the CPU's config setting
void cpu_set_config(riscv_cpu_t* const cpu, rsk_config_t config);

// Have the CPU load a byte value
byte cpu_load_byte(const riscv_cpu_t* const cpu, dword address);

// Have the CPU store a byte value
void cpu_store_byte(const riscv_cpu_t* const cpu, dword address, byte value);

// Have the CPU load a hword value
hword cpu_load_hword(const riscv_cpu_t* const cpu, dword address);

// Have the CPU store a hword value
void cpu_store_hword(const riscv_cpu_t* const cpu, dword address, hword value);

// Have the CPU load a word value
word cpu_load_word(const riscv_cpu_t* const cpu, dword address);

// Have the CPU store a word value
void cpu_store_word(const riscv_cpu_t* const cpu, dword address, word value);

// Have the CPU load a dword value
dword cpu_load_dword(const riscv_cpu_t* const cpu, dword address);

// Have the CPU store a dword value
void cpu_store_dword(const riscv_cpu_t* const cpu, dword address, dword value);

// Get the CPU's pc value
dword cpu_get_pc(const riscv_cpu_t* const cpu);

// Set the CPU's pc value
void cpu_set_pc(riscv_cpu_t* const cpu, dword address);

// Safely access a register value. Calls the host service 'panic' if an illegal access occurs.
dword cpu_read_register(const riscv_cpu_t* const cpu, int index);

// Safely write a value to a register. Calls the host service 'panic' if an illegal access occurs.
void cpu_write_register(const riscv_cpu_t* const cpu, int index, dword value);

// Have the CPU process a RISC-V signal
void cpu_process_signal(riscv_cpu_t* const cpu, rsk_signal_t signal);

// Have the CPU report the current state of the CPU to the host
void cpu_log_trace(const riscv_cpu_t* const cpu);

// Have the CPU log a message with the host
void cpu_log_message(const riscv_cpu_t* cpu, const char* const message);

// Have the CPU send a panic message to the host
void cpu_panic(const riscv_cpu_t* const cpu, const char* message);

// Have the CPU fill the provided stats struct with its current statistics
void cpu_fill_stats(const riscv_cpu_t* const cpu, rsk_stat_t* stats);

// Disassemble the current instruction
void cpu_disassemble(const riscv_cpu_t* const cpu, char* buffer, size_t buffer_size);

// Execute the instruction at pc and return 0 if ebreak was hit
int cpu_execute(riscv_cpu_t* const cpu);

#endif