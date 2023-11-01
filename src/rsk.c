/*
 - Joshua Douglas
 - Cps 310 Microprocessor Architecture
 - Fall 2023
*/

/* RISC-V Sim Kernel/Shell Interface definition */
#include "rskapi.h"

// RISC-V Sim Info
char* riscv_sim_info[] = {
    "author=jdoug344",
    "api=1.0",
    "mockup",
    NULL
};

struct rvi64_cpu {
    // Running
    int is_running;

    // Configuration
    rsk_config_t config;
    rsk_host_services_t host;
    rsk_stat_t stats;

    // Registers (x[0] is included to keep the index values consistent with the register names. It should not be written to and should always read 0)
    dword x[32];
    dword pc;
} cpu;


char** rsk_info(void) {
    return riscv_sim_info;
}

void rsk_disasm(dword address, dword instruction, char* buffer, size_t size) {
    // TODO: resize the buffer and fill with the disassembled instruction
}

void rsk_init(const rsk_host_services_t* services) {
    cpu.is_running = 0;

    // Zero register values
    for (int i = 0; i < sizeof(cpu.x) / sizeof(cpu.x[0]); i++) cpu.x[i] = 0;

    // Clear configuration flags
    cpu.config = rc_nothing;

    // Bind to provided services
    cpu.host = *services;

    // Zero CPU statistics
    cpu.stats.instructions = 0;
    cpu.stats.loads = 0;
    cpu.stats.load_misses = 0;
    cpu.stats.stores = 0;
    cpu.stats.store_misses = 0;

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
    stats->loads = cpu.stats.loads;
    stats->load_misses = cpu.stats.load_misses;
    stats->stores = cpu.stats.stores;
    stats->store_misses = cpu.stats.store_misses;
}

dword rsk_reg_get(int index) {
    if (index < 0 || index > 31) return -1;

    if (0 == index) return 0;
    return cpu.x[index];
}

void rsk_reg_set(int index, dword value) {
    if (index < 1 || index > 31) return;

    cpu.x[index] = value;
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
    cpu.stats.instructions += cycles;
    return cycles;
}
