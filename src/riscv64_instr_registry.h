#ifndef RISCV64_INSTRUCTION_REGISTRY
#define RISCV64_INSTRUCTION_REGISTRY

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// ---------- Fundamental unit alias typedefs ----------

// An 8 bit unsigned value
typedef uint8_t byte;

// An 8 bit signed value
typedef int8_t sbyte;

// A 16 bit unsigned value
typedef uint16_t hword;

// A 16 bit signed value
typedef int16_t shword;

// A 32 bit unsigned value
typedef uint32_t word;

// A 32 bit signed value
typedef int32_t sword;

// A 64 bit unsigned value
typedef uint64_t dword;

// A 64 bit signed value
typedef int64_t sdword;

// ---------- RISC-V Instruction Type Registry ----------

// A set of registered RISC-V instruction types, used for decoding, disassembly, and execution
typedef struct riscv64_instruction_type {
	// Mask of the required fields for this instruction type
	dword mask;

	// Required bits within the mask for this instruction type
	dword required_bits;

	// Disassemble an instruction of this type, putting the result into the provided string buffer. Returns the length of the complete disassembled instruction. If the buffer is not large enough to hold the instruction, the resulting string will be terminated early.
	size_t (*disassemble)(dword instruction, char* buffer, size_t buffer_size);

	// Execute an instruction of this type
	void (*execute)(dword instruction);
} instruction_type_t;

// Registry is an array of instruction type definitions
typedef instruction_type_t* registry_t;

// Determine the type of an instruction. Returns a pointer to the registry entry for the instruction's type, or NULL if the instruction does not match a registered type.
instruction_type_t* registry_lookup(const registry_t const registry, dword instruction) {
	// TODO: lookup instruction
}

// Resize the instruction registry for a specific number of additional instructions
registry_t registry_resize(size_t* registry_count, registry_t registry, const size_t addition_count) {
	if (NULL == registry) {
		registry = (registry_t) malloc(sizeof(struct riscv64_instruction_type) * addition_count);
		*registry_count = (NULL == registry) ? 0 : addition_count;
	} else {
		size_t re_count = *registry_count + addition_count;
		struct riscv64_instruction_type* re_registry = (struct riscv64_instruction_type*) realloc(registry, sizeof(struct riscv64_instruction_type) * re_count);

		if (NULL == re_registry) {
			free(registry);
			registry = NULL;
		} else {
			registry = re_registry;
		}

		*registry_count = (NULL == registry) ? 0 : re_count;
	}
	
	return registry;
}

// ---------- Disassembly/Execution Function Names ----------

#define DISASSEMBLY_DEF(name)   size_t rv64_disassembly_##name(dword instruction, char* buffer, size_t buffer_size)
#define EXECUTION_DEF(name)     void rv64_execution_##name(dword instruction)

#define INSTRUCTION_LINKS(name) .disassemble = rv64_disassembly_##name##, .execute = rv64_execution_##name

// ---------- Generic 32-bit Mask Construction ----------

#define MASK_FROM_31 0xffffffff
#define MASK_FROM_30 0x7fffffff
#define MASK_FROM_29 0x3fffffff
#define MASK_FROM_28 0x1fffffff
#define MASK_FROM_27 0xfffffff
#define MASK_FROM_26 0x7ffffff
#define MASK_FROM_25 0x3ffffff
#define MASK_FROM_24 0x1ffffff
#define MASK_FROM_23 0xffffff
#define MASK_FROM_22 0x7fffff
#define MASK_FROM_21 0x3fffff
#define MASK_FROM_20 0x1fffff
#define MASK_FROM_19 0xfffff
#define MASK_FROM_18 0x7ffff
#define MASK_FROM_17 0x3ffff
#define MASK_FROM_16 0x1ffff
#define MASK_FROM_15 0xffff
#define MASK_FROM_14 0x7fff
#define MASK_FROM_13 0x3fff
#define MASK_FROM_12 0x1fff
#define MASK_FROM_11 0xfff
#define MASK_FROM_10 0x7ff
#define MASK_FROM_9  0x3ff
#define MASK_FROM_8  0x1ff
#define MASK_FROM_7  0xff
#define MASK_FROM_6  0x7f
#define MASK_FROM_5  0x3f
#define MASK_FROM_4  0x1f
#define MASK_FROM_3  0xf
#define MASK_FROM_2  0x7
#define MASK_FROM_1  0x3
#define MASK_FROM_0  0x1

#define MASK_TO_31 0x8
#define MASK_TO_30 0xc
#define MASK_TO_29 0xe
#define MASK_TO_28 0xf
#define MASK_TO_27 0xf8
#define MASK_TO_26 0xfc
#define MASK_TO_25 0xfe
#define MASK_TO_24 0xff
#define MASK_TO_23 0xff8
#define MASK_TO_22 0xffc
#define MASK_TO_21 0xffe
#define MASK_TO_20 0xfff
#define MASK_TO_19 0xfff8
#define MASK_TO_18 0xfffc
#define MASK_TO_17 0xfffe
#define MASK_TO_16 0xffff
#define MASK_TO_15 0xffff8
#define MASK_TO_14 0xffffc
#define MASK_TO_13 0xffffe
#define MASK_TO_12 0xfffff
#define MASK_TO_11 0xfffff8
#define MASK_TO_10 0xfffffc
#define MASK_TO_9  0xfffffe
#define MASK_TO_8  0xffffff
#define MASK_TO_7  0xffffff8
#define MASK_TO_6  0xffffffc
#define MASK_TO_5  0xffffffe
#define MASK_TO_4  0xfffffff
#define MASK_TO_3  0xfffffff8
#define MASK_TO_2  0xfffffffc
#define MASK_TO_1  0xfffffffe
#define MASK_TO_0  0xffffffff

// ---------- Masking Constants ----------

#define MASK_FUNCT7  0xfe000000
#define FUNCT7(bits) ((0b##bits << 25) & MASK_FUNCT7)

#define MASK_RS2     0x1f00000
#define RS2(bits)    ((0b##bits << 20) & MASK_RS2)

#define MASK_RS1     0xf8000
#define RS1(bits)    ((0b##bits << 15) & MASK_RS1)

#define MASK_FUNCT3  0x7000
#define FUNCT3(bits) ((0b##bits << 12) & MASK_FUNCT3)

#define MASK_RD      0xf80
#define RD(bits)     ((0b##bits << 7) & MASK_RD)

#define MASK_OPCODE  0x7f
#define OPCODE(bits) (0b##bits & MASK_OPCODE)

#endif