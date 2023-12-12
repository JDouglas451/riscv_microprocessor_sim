# RISC-V Simulator
Author: Joshua Douglas (jdoug344)

## Building RSKAPI
To build the mockup with gcc test flags, run the following:
```
$ make debug
```

To build the release version, run the following:
```
$ make
```

The program can be run with the following command after building:
```
$ make run [FILE=filename]
```
This will run rsh.py with the librsk.so library. If the "mockup" has been included in the information array, then no other parameters are needed. If the program does not specify the mockup phase, the FILE argument can be filled to provide an ELF file to run. Without a file, only the simulator info will be printed.

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
