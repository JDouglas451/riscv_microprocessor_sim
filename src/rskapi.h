/*
 - RISC-V Sim Kernel API
 - Defines interface between RISC-V Sim kernels (CPUs) and hosts (systems)
 - (c) 2016-2023, Bob Jones University
*/

#ifndef RISCV_SIM_KERNEL_API
#define RISCV_SIM_KERNEL_API

#include <stddef.h>
#include <stdint.h>

#include "riscv64.h"

// rskAPI functions must be exported with "C" linkage, even if implemented in C++
#ifdef __cplusplus
extern "C" {
#endif

// ----------------- Kernel API ---------------- //
// (functions available to the host)

// Return a pointer to a NULL-terminated list of NUL-terminated C-strings describing the kernel's features
const char* const* rsk_info(void);

// Disassemble a given 64-bit RISC-V <instruction> into the string buffer provided. The <address> where the instruction is found in memory is only required to calculate branch targets.
void rsk_disasm(dword instruction, char* buffer, size_t size);

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

// Start the CPU running for <cycles> instructions, or until EBREAK if <cycles> is 0. Returns the number of instructions executed, which should match <cycles>, unless <cycles> was 0.
int rsk_cpu_run(int cycles);

#ifdef __cplusplus
}
#endif

#endif