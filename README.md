# RISC-V Simulator
Author: Joshua Douglas (jdoug344)

## Building RSKAPI
Before making use of the Makefile, it may be necessary to modify the paths at the head of the file (specifically, the paths to the riscv toolchain).

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
