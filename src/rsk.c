/*
 - Joshua Douglas
 - Cps 310 Microprocessor Architecture
 - Fall 2023
*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

// RISC-V Sim Kernel/Shell Interface definition
#include "rskapi.h"

// Simulator data structures, typedefs, and instructions
#include "riscv64.h"

// ---------- Simulator Information ----------

const char* const riscv_sim_info[] = {
    "author=jdoug344",
    "api=1.0",
    NULL
};

riscv_cpu_t* cpu = NULL;

// ---------- API Function Definitions ----------

const char* const* rsk_info(void) {
    return riscv_sim_info;
}

void rsk_disasm(dword instruction, char* buffer, size_t size) {
	cpu_disassemble(cpu, buffer, size);
}

void rsk_init(const rsk_host_services_t* services) {
    cpu = cpu_init(cpu, services);
    cpu_log_message(cpu, "CPU initialized");
}

void rsk_config_set(rsk_config_t flags) {
    cpu_set_config(cpu, flags);
}

rsk_config_t rsk_config_get(void) {
    return cpu_get_config(cpu);
}

void rsk_stats_report(rsk_stat_t* stats) {
    cpu_fill_stats(cpu, stats);
}

dword rsk_reg_get(int index) {
    return cpu_read_register(cpu, index);
}

void rsk_reg_set(int index, dword value) {
    cpu_write_register(cpu, index, value);
}

dword rsk_pc_get(void) {
    return cpu_get_pc(cpu);
}

void rsk_pc_set(dword value) {
    cpu_set_pc(cpu, value);
}

int rsk_cpu_running(void) {
    return cpu_is_running(cpu);
}

void rsk_cpu_signal(rsk_signal_t signal) {
    cpu_process_signal(cpu, signal);
}

int rsk_cpu_run(int cycles) {
	int count = 0;

	while (cpu_execute(cpu)) count++;
	if (count > 0) count++; // include ebreak in count
	
    return count;
}
