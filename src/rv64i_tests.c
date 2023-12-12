#include "riscv64_testing.h"

int main() {
    riscv_cpu_t* cpu = cpu_init(NULL, &test_services);
    if (NULL == cpu) return 1;

    
}
