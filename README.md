# RISC-V Simulator
Author: Joshua Douglas (jdoug344)

## Building RSKAPI
Before making use of the Makefile, it may be necessary to modify the paths at the head of the file (specifically, the paths to the riscv toolchain). This is the toolchain I used: https://github.com/riscv-collab/riscv-gnu-toolchain. Regardless of the toolchain you use, it will probably be necessary to modify a few of the path variables at the start of the Makefile.

To build the mockup with gcc test flags, run the following:
```
$ make debug
```

To build the release version, run the following:
```
$ make
```

## Convenient Commands
The program can be run with the following command after building:
```
$ make run [FILE=filename]
```
This will run rsh.py with the librsk.so library. If the "mockup" has been included in the information array, then no other parameters are needed. If the program does not specify the mockup phase, the FILE argument can be filled to provide an ELF file to run. Without a file, only the simulator info will be printed.

Commands to run objdump and readelf are also provided:
```
$ make objdump FILE=filename
$ make readelf FILE=filename
```
Note that the FILE argument is required for these commands.

## API Tests
A few tests are included with the project in the **tests/** folder. They can be built with:
```
$ make tests
```
Once built, their ELF files will appear in the **build/tests/** folder.

To test the instruction decoding and disassembly, run the following:
```
$ make isa_test
```
The tests will be built and run automatically, and any instructions that do not decode or disassemble correctly will be reported.

## Project Structure
The RISC-V simulator is designed so that only the CPU struct and functions dealing with CPU structs are exposed to external files. For testing, a testing header exposes the internal structs and functions of the simulator. It exists on the same level as 'rsk.c'. 

### CPU Struct
The CPU struct contains within itself references to several structs defined by the API, as well as an instruction registry struct and the PC and registers. The CPU has register access methods that treat the zero register correctly, as well as getter and setter methods for all of it's non-struct properties.

### Instruction Registry
The instruction registry struct contains an array of instruction type structs. Each of these has two sets of bits for matching instructions, as well as the name, disassembly function, and execution function of the instruction. The registry can be added to without the need to copy structs (it is resized to fit the added instructions, which are stored as pointers). A search method is defined to make matching instructions to types easy. In the future, I would like to make the process of adding instruction types easier and more unified (because C doesn't have lambdas, the disassembly and execution functions must be separated from the rest of the struct's declaration, meaning that two places must be referenced to see all of the implementation details of an instruction type.)

### Metaprogramming
The instruction definitions make extensive use of C preprocessor definitions to create what is essentially a minor domain specific language for implementing disassembly and execution functions. Preprocessor functions for bit manipulation are also included to make instruction matching easier. A summary of these preprocessor functions is given below. (See **riscv64.c** for the definitions in the codebase, and any of the **rv64\*_instr.h** header files for examples of their usage)

Bit manipulation can be arduous, and typing binary or hexadecimal fields in by hand is prone to error. To overcome this, the following preprocessor definitions make bit mask construction much easier to read and write:
```
#define BITSMASK(high, low) ((__UINT64_MAX__ << (high + 1)) ^ (__UINT64_MAX__ << low))
#define BITMASK(bit)        (1 << bit)

#define INSTR_SIGN   BITMASK(31)

#define INSTR_FUNCT7 BITSMASK(31, 25)
#define FUNCT7(bits) ((0b##bits << 25) & INSTR_FUNCT7)

#define INSTR_RS2    BITSMASK(24, 20)
#define RS2(bits)    ((0b##bits << 20) & INSTR_RS2)

#define INSTR_RS1    BITSMASK(19, 15)
#define RS1(bits)    ((0b##bits << 15) & INSTR_RS1)

#define INSTR_FUNCT3 BITSMASK(14, 12)
#define FUNCT3(bits) ((0b##bits << 12) & INSTR_FUNCT3)

#define INSTR_RD     BITSMASK(11, 7)
#define RD(bits)     ((0b##bits << 7) & INSTR_RD)

#define INSTR_OPCODE BITSMASK(6, 0)
#define OPCODE(bits) (0b##bits & INSTR_OPCODE)
```

Typing the same function defnition for each instruction would be inefficient, and would make the code more daunting for unfamilier reviewers. These preprocessor definitions shorten the actual function declarations for disassembly and execute functions, requiring only the information necessary to identify the purpose and instruction of the operation, which is much easier on the eyes. Since the instruction names are now hidden, it is also necessary to provide a way to fill instruction structs with the correct functions:
```
#define DISASM_DEF(name) size_t z_disasm_##name(riscv_cpu_t* const cpu, word instr, char* buffer, size_t buffer_size)
#define EXEC_DEF(name)   void z_exec_##name(riscv_cpu_t* const cpu, word instr, int* updated_pc)

// add function references to structs
#define INSTR_LINKS(name) .disassemble = z_disasm_##name , .execute = z_exec_##name
```

In the execution functions where many relatively long functions may be used often, preprocessor definitions can significantly reduce the apparent complexity of the code. In order to simplify the instruction execution code, the following preprocessor definitions have been defined:
```
#define GET_FUNCT7 mask_instr_funct7(instr)
#define GET_RS2    mask_instr_rs2(instr)
#define GET_RS1    mask_instr_rs1(instr)
#define GET_FUNCT3 mask_instr_funct3(instr)
#define GET_RD     mask_instr_rd(instr)
#define GET_OPCODE mask_instr_opcode(instr)

#define READ_REG(index)         cpu_read_register(cpu, index)
#define WRITE_REG(index, value) cpu_write_register(cpu, index, value)

// set flag as well so pc won't be changed twice
#define SET_PC(addr) *updated_pc = 1; cpu_set_pc(cpu, addr)
#define GET_PC       cpu_get_pc(cpu)

#define STORE_BYTE(addr, value)  cpu_store_byte(cpu, addr, value)
#define STORE_HWORD(addr, value) cpu_store_hword(cpu, addr, value)
#define STORE_WORD(addr, value)  cpu_store_word(cpu, addr, value)
#define STORE_DWORD(addr, value) cpu_store_dword(cpu, addr, value)

#define LOAD_BYTE(addr)  cpu_load_byte(cpu, addr)
#define LOAD_HWORD(addr) cpu_load_hword(cpu, addr)
#define LOAD_WORD(addr)  cpu_load_word(cpu, addr)
#define LOAD_DWORD(addr) cpu_load_dword(cpu, addr)
```

