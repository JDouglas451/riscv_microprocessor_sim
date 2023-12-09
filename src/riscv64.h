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

riscv64_cpu cpu;

// ---------- API Function Definitions ----------

const char* const* rsk_info(void) {
    return riscv_sim_info;
}

void rsk_disasm(dword instruction, char* buffer, size_t size) {
    // TODO: resize the buffer and fill with the disassembled instruction
}

void rsk_init(const rsk_host_services_t* services) {
    cpu_init(&cpu, services);

    // Log initialization
    cpu.host.log_msg("CPU initialized");
}

void rsk_config_set(rsk_config_t flags) {
    cpu.config = flags;
}

rsk_config_t rsk_config_get(void) {
    return cpu.config;
}

void rsk_stats_report(rsk_stat_t* stats) {
    stats->instructions = cpu.stats.instructions;
    stats->loads        = cpu.stats.loads;
    stats->load_misses  = cpu.stats.load_misses;
    stats->stores       = cpu.stats.stores;
    stats->store_misses = cpu.stats.store_misses;
}

dword rsk_reg_get(int index) {
    cpu_read_register(&cpu, index);
}

void rsk_reg_set(int index, dword value) {
    cpu_write_register(&cpu, index, value);
}

dword rsk_pc_get(void) {
    return cpu.pc;
}

void rsk_pc_set(dword value) {
    cpu.pc = value;
}

int rsk_cpu_running(void) {
    return cpu.is_running;
}

void rsk_cpu_signal(rsk_signal_t signal) {
    if (signal == rs_halt) {
        cpu.is_running = 0;
    }
}

int rsk_cpu_run(int cycles) {
	// TODO: run for cycles or until ebreak
    cpu.stats.instructions += cycles;
    return cycles;
}
