#ifndef SIM_RISCV64_TESTING
#define SIM_RISCV64_TESTING

#include "rskapi.h"
#include "riscv64.h"

// ---------- Test Services ----------

dword z_test_load_dword(dword address) { return 0; }
void z_test_store_dword(dword address, dword value) { return; }

word z_test_load_word(dword address) { return 0; }
void z_test_store_word(dword address, word value) { return; }

hword z_test_load_hword(dword address) { return 0; }
void z_test_store_hword(dword address, hword value) { return; }

byte z_test_load_byte(dword address) { return 0; }
void z_test_store_byte(dword address, byte value) { return; }

void z_test_log_trace(unsigned step, dword pc, dword *registers) { return; }

void z_test_log_message(const char *msg) { fprintf(stdout, msg); }
void z_test_panic(const char *msg)       { fprintf(stderr, msg); }

rsk_host_services_t test_services = {
    .mem_load_dword =  z_test_load_dword,
    .mem_store_dword = z_test_store_dword,
    .mem_load_word =   z_test_load_word,
    .mem_store_word =  z_test_store_word,
    .mem_load_hword =  z_test_load_hword,
    .mem_store_hword = z_test_store_hword,
    .mem_load_byte =   z_test_load_byte,
    .mem_store_byte =  z_test_store_byte,
    .log_trace =       z_test_log_trace,
    .log_msg =         z_test_log_message,
    .panic =           z_test_panic
};

#endif