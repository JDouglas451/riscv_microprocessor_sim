# directories
SRC_DIR := src/
BUILD_DIR := build/
TEST_SRC_DIR := tests/
TEST_BUILD_DIR := build/tests/

# default gcc flags
CFLAGS = -DNDEBUG

# riscv compilation and linking
RV_DIR := /opt/riscv/bin/
RV_LINKER := linker.ld
RV_COMPAT := compat.txt

RV_GCC_FLAGS := -nostdlib -mno-shorten-memrefs -fno-builtin -nostartfiles -nodefaultlibs -march=rv64imd
RV_GCC := $(RV_DIR)riscv64-unknown-linux-gnu-gcc
RV_LD := $(RV_DIR)riscv64-unknown-linux-gnu-ld
RV_OBJCPY := $(RV_DIR)riscv64-unknown-linux-gnu-objcopy

# prevent make taking too much initiative when building tests
.SUFFIXES:
.PHONY: clean run debug tests objdump readelf

librsk.so: riscv64.o
	gcc $(CFLAGS) -fPIC -c -o $(BUILD_DIR)rskapi.o $(SRC_DIR)rsk.c
	gcc $(CFLAGS) -shared -o $(BUILD_DIR)librsk.so $(BUILD_DIR)rskapi.o $(BUILD_DIR)riscv64.o

	@find "$(TEST_SRC_DIR)" -maxdepth 1 -type f -not -name "*.s" -delete

riscv64.o:
	gcc $(CFLAGS) -fPIC -c -o $(BUILD_DIR)riscv64.o $(SRC_DIR)riscv64.c

# debug gcc flags
debug: CFLAGS = -g -Wall -Werror
debug: librsk.so

# pattern rule for test files
$(TEST_SRC_DIR)%: $(TEST_SRC_DIR)%.s
	@echo "Compiling and linking '$^'..."

	$(RV_GCC) -o "$@.o" -c "$^" $(RV_GCC_FLAGS)
	$(RV_LD) -T "$(RV_LINKER)" -Ttext=0x1000 -n -e _start -o "$@.exe" "$@.o"
	$(RV_OBJCPY) --add-section .riscvsim="$(RV_COMPAT)" --set-section-flags .riscvsim=noload "$@.exe"

	@mv "$@.exe" $(TEST_BUILD_DIR)
	@find "$(TEST_SRC_DIR)" -maxdepth 1 -type f -not -name "*.s" -delete

tests: $(patsubst %.s,%,$(wildcard $(TEST_SRC_DIR)*.s))

rv64i_tests.o:
	gcc $(CFLAGS) $(SRC_DIR)rv64i_tests.c -o $(BUILD_DIR)rv64i_tests.o $(BUILD_DIR)riscv64.o
	@echo "---------- Testing RV64I instructions ----------"
	@chmod +x $(BUILD_DIR)rv64i_tests.o && ./$(BUILD_DIR)rv64i_tests.o
	@echo "------------- RV64I tests complete -------------"

isa_test: CFLAGS = -g -Wall -Werror
isa_test: riscv64.o rv64i_tests.o

objdump:
	$(RV_DIR)riscv64-unknown-linux-gnu-objdump -d -Mno-aliases $(FILE)

readelf:
	$(RV_DIR)riscv64-unknown-linux-gnu-readelf -a $(FILE)

# run the python host
run: librsk.so
	@python src/rsh.py build/librsk.so $(FILE)

clean:
	@find "$(TEST_BUILD_DIR)" -maxdepth 1 -type f -delete
	@find "$(BUILD_DIR)" -maxdepth 1 -type f -delete
