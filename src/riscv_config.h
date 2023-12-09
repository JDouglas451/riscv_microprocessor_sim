#ifndef RISCV_SIM_CONFIG
#define RISCV_SIM_CONFIG

#include <stddef.h>
#include <stdint.h>

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

#endif