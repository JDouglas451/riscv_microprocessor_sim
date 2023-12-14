#ifndef SIM_RISCV64
#define SIM_RISCV64

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "rskapi.h"

// ---------- CPU Data Structures ----------

// RISC-V bit CPU struct
typedef struct riscv64_cpu riscv_cpu_t;

// ---------- CPU Methods ----------

// Initialize the CPU with default values and the provided host services
riscv_cpu_t* cpu_init(riscv_cpu_t* cpu, const rsk_host_services_t* const services);

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
dword cpu_read_register(const riscv_cpu_t* const cpu, byte index);

// Safely write a value to a register. Calls the host service 'panic' if an illegal access occurs.
void cpu_write_register(riscv_cpu_t* const cpu, byte index, dword value);

// Have the CPU process a RISC-V signal
void cpu_process_signal(riscv_cpu_t* const cpu, rsk_signal_t signal);

// Have the CPU report the current state of the CPU to the host
void cpu_log_trace(riscv_cpu_t* const cpu);

// Have the CPU log a message with the host
void cpu_log_message(const riscv_cpu_t* cpu, const char* const message);

// Have the CPU send a panic message to the host
void cpu_panic(const riscv_cpu_t* const cpu, const char* message);

// Have the CPU fill the provided stats struct with its current statistics
void cpu_fill_stats(const riscv_cpu_t* const cpu, rsk_stat_t* stats);

// Get the number of instructions executed by the cpu since initialization
unsigned int cpu_stat_instructions(riscv_cpu_t* const cpu);

// Return the instruction name of an encoded instruction
const char* const cpu_identify_instr(riscv_cpu_t* const cpu, word instr);

// Disassemble the provided instruction
void cpu_disassemble_instr(riscv_cpu_t* const cpu, char* buffer, size_t buffer_size, word instr);

// Disassemble the current instruction
void cpu_disassemble(riscv_cpu_t* const cpu, char* buffer, size_t buffer_size);

// Execute the instruction at pc and return 0 if ebreak was hit or an error occurred
int cpu_execute(riscv_cpu_t* const cpu);

#endif