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

RV_GCC_FLAGS := -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -march=rv64imd
RV_GCC := $(RV_DIR)riscv64-unknown-linux-gnu-gcc $(RV_GCC_FLAGS)

RV_LD := $(RV_DIR)riscv64-unknown-linux-gnu-ld -T "$(RV_LINKER)" -n -e _start
RV_OBJCPY := $(RV_DIR)riscv64-unknown-linux-gnu-objcopy --add-section .riscvsim="$(RV_COMPAT)" --set-section-flags .riscvsim=noload

# prevent make taking too much initiative when building tests
.SUFFIXES:

librsk.so: riscv64.o
	gcc $(CFLAGS) -fPIC -c -o $(BUILD_DIR)rskapi.o $(SRC_DIR)rsk.c
	gcc $(CFLAGS) -shared -o $(BUILD_DIR)librsk.so $(BUILD_DIR)rskapi.o $(BUILD_DIR)riscv64.o

riscv64.o:
	gcc $(CFLAGS) -fPIC -c -o $(BUILD_DIR)riscv64.o $(SRC_DIR)riscv64.c

# debug gcc flags
debug: CFLAGS = -g -Wall -Werror
debug: librsk.so

# pattern rule for test files
$(TEST_SRC_DIR)%: $(TEST_SRC_DIR)%.s
	@echo "Compiling and linking '$^'..."
	$(RV_GCC) -o "$@.o" -c "$^"
	$(RV_LD) -o "$@.l" "$@.o"
	$(RV_OBJCPY) "$@.l"
	@cp $(TEST_SRC_DIR)*.o $(TEST_BUILD_DIR)
	@rm $(TEST_SRC_DIR)*.[lo] || true

tests: $(patsubst %.s,%,$(wildcard $(TEST_SRC_DIR)*.s))

rv64i_tests.o:
	gcc $(CFLAGS) $(SRC_DIR)rv64i_tests.c -o $(BUILD_DIR)rv64i_tests.o $(BUILD_DIR)riscv64.o
	@echo "---------- Testing RV64I instructions ----------"
	@chmod +x $(BUILD_DIR)rv64i_tests.o && ./$(BUILD_DIR)rv64i_tests.o
	@echo "------------- RV64I tests complete -------------"

isa_test: CFLAGS = -g -Wall -Werror
isa_test: riscv64.o rv64i_tests.o

objdump:
	$(RV_DIR)riscv64-unknown-linux-gnu-objdump -d -M no-aliases $(FILE)

readelf:
	$(RV_DIR)riscv64-unknown-linux-gnu-readelf -a $(FILE)

# run the python host
run: librsk.so
	@python src/rsh.py build/librsk.so $(FILE)

clean:
	@rm $(TEST_BUILD_DIR)* || true
	@rm $(BUILD_DIR)*.* || true
