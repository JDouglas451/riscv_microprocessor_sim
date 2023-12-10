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

// Simulator data structures and typedefs
#include "riscv_config.h"
#include "riscv64_cpu.h"

// ---------- Simulator Information ----------

const char* const riscv_sim_info[] = {
    "author=jdoug344",
    "api=1.0",
    "mockup",
    NULL
};

riscv_cpu_t sim_cpu;

// ---------- API Function Definitions ----------

const char* const* rsk_info(void) {
    return riscv_sim_info;
}

void rsk_disasm(dword instruction, char* buffer, size_t size) {
	cpu_disassemble(&sim_cpu, buffer, size);
}

void rsk_init(const rsk_host_services_t* services) {
    cpu_init(&sim_cpu, services);
    sim_cpu.host.log_msg("CPU initialized");
}

void rsk_config_set(rsk_config_t flags) {
    sim_cpu.config = flags;
}

rsk_config_t rsk_config_get(void) {
    return sim_cpu.config;
}

void rsk_stats_report(rsk_stat_t* stats) {
    stats->instructions = sim_cpu.stats.instructions;
    stats->loads        = sim_cpu.stats.loads;
    stats->load_misses  = sim_cpu.stats.load_misses;
    stats->stores       = sim_cpu.stats.stores;
    stats->store_misses = sim_cpu.stats.store_misses;
}

dword rsk_reg_get(int index) {
    cpu_read_register(&sim_cpu, index);
}

void rsk_reg_set(int index, dword value) {
    cpu_write_register(&sim_cpu, index, value);
}

dword rsk_pc_get(void) {
    return sim_cpu.pc;
}

void rsk_pc_set(dword value) {
    sim_cpu.pc = value;
}

int rsk_cpu_running(void) {
    return sim_cpu.is_running;
}

void rsk_cpu_signal(rsk_signal_t signal) {
    if (signal == rs_halt) {
        sim_cpu.is_running = 0;
    }
}

int rsk_cpu_run(int cycles) {
	int count = 0;

	while (cpu_execute(&sim_cpu)) count++;
	if (count > 0) count++; // include ebreak in count
	
    return count;
}
