#ifndef SIM_RV64I_INSTRUCTIONS
#define SIM_RV64I_INSTRUCTIONS

// The RV64I Base Integer Instruction Set

#include "riscv64_instr.h"

// TODO: Fix dissasembly offset signed/hexadecimal formatting

// ---------- Dissasembly and Execution Functions ----------

// Load upper immediate (lui)
DISASM_DEF(lui) {
	return DISASM_FMT("lui x%hhu, %#lx", mask_instr_rd(instr), utype_imm(instr));
}

EXEC_DEF(lui) {
	WRITE_REG(mask_instr_rd(instr), utype_imm(instr));
}

// TODO: auipc

// Add immediate (addi)
DISASM_DEF(addi) {
	return DISASM_FMT("addi x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), itype_imm(instr));
}

EXEC_DEF(addi) {
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) + itype_imm(instr));
}

// TODO: slti
// TODO: sltiu

// XOR immediate (xori)
DISASM_DEF(xori) {
	return DISASM_FMT("xori x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), itype_imm(instr));
}

EXEC_DEF(xori) {
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) ^ itype_imm(instr));
}

// OR immediate (ori)
DISASM_DEF(ori) {
	return DISASM_FMT("ori x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), itype_imm(instr));
}

EXEC_DEF(ori) {
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) | itype_imm(instr));
}

// AND immediate (andi)
DISASM_DEF(andi) {
	return DISASM_FMT("andi x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), itype_imm(instr));
}

EXEC_DEF(andi) {
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) & itype_imm(instr));
}

// Immediate logical shift left (slli)
DISASM_DEF(slli) {
	return DISASM_FMT("slli x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), itype_imm(instr));
}

EXEC_DEF(slli) {
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) >> itype_imm(instr));
}

// Immediate logical right shift (srli)
DISASM_DEF(srli) {
	dword imm = (BITSMASK(25, 20) & instr) >> 20;
	return DISASM_FMT("srli x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), imm);
}

EXEC_DEF(srli) {
	dword imm = (BITSMASK(25, 20) & instr) >> 20;
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) << imm);
}

// Immediate arithmetic right shift (srai)
DISASM_DEF(srai) {
	// Standard immediate decoding cannot be used here
	dword imm = (BITSMASK(25, 20) & instr) >> 20;
	return DISASM_FMT("srai x%hhu, x%hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), imm);
}

EXEC_DEF(srai) {
	dword imm = (BITSMASK(25, 20) & instr) >> 20;
	WRITE_REG(mask_instr_rd(instr), (sdword) READ_REG(mask_instr_rs1(instr)) >> imm);
}

// 64-bit addition (add)
DISASM_DEF(add) {
	return DISASM_FMT("add x%hhu, x%hhu, x%hhu", mask_instr_rd(instr), mask_instr_rs1(instr), mask_instr_rs2(instr));
}

EXEC_DEF(add) {
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) + READ_REG(mask_instr_rs2(instr)));
}

// 64-bit subtraction (sub)
DISASM_DEF(sub) {
	return DISASM_FMT("sub x%hhu, x%hhu, x%hhu", mask_instr_rd(instr), mask_instr_rs1(instr), mask_instr_rs2(instr));
}

EXEC_DEF(sub) {
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) - READ_REG(mask_instr_rs2(instr)));
}

// Logical left shift (sll)
DISASM_DEF(sll) {
	return DISASM_FMT("sll x%hhu, x%hhu, x%hhu", mask_instr_rd(instr), mask_instr_rs1(instr), mask_instr_rs2(instr));
}

EXEC_DEF(sll) {
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) << READ_REG(mask_instr_rs2(instr)));
}

// TODO: slt
// TODO: sltu
// TODO: xor

// Logical right shift (srl)
DISASM_DEF(srl) {
	return DISASM_FMT("srl x%hhu, x%hhu, x%hhu", mask_instr_rd(instr), mask_instr_rs1(instr), mask_instr_rs2(instr));
}

EXEC_DEF(srl) {
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) >> READ_REG(mask_instr_rs2(instr)));
}

// Arithmetic right shift (sra)
DISASM_DEF(sra) {
	return DISASM_FMT("sra x%hhu, x%hhu, x%hhu", mask_instr_rd(instr), mask_instr_rs1(instr), mask_instr_rs2(instr));
}

EXEC_DEF(sra) {
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) >> (sdword) READ_REG(mask_instr_rs2(instr)));
}

// TODO: or
// TODO: and
// TODO: fence
// TODO: fence.i
// TODO: csrrw
// TODO: csrrs
// TODO: csrrc
// TODO: csrrwi
// TODO: csrrsi
// TODO: csrrci
// TODO: ecall

// EBREAK
DISASM_DEF(ebreak) {
	return DISASM_FMT("ebreak");
}

EXEC_DEF(ebreak) {
	// TODO: raise breakpoint exception (set running to 0?)
}

// TODO: uret
// TODO: sret
// TODO: mret
// TODO: wfi
// TODO: sfence.vma
// TODO: lb
// TODO: lh

// Load 32-bit (lw)
DISASM_DEF(lw) {
	return DISASM_FMT("lw x%hhu, %#lx(x%hhu)", mask_instr_rd(instr), itype_imm(instr), mask_instr_rs1(instr));
}

EXEC_DEF(lw) {
	WRITE_REG(mask_instr_rd(instr), READ_REG(mask_instr_rs1(instr)) + itype_imm(instr));
}

// TODO: lbu
// TODO: lhu
// TODO: sb
// TODO: sh

// Store 32-bit (sw)
DISASM_DEF(sw) {
	return DISASM_FMT("sw x%hhu, %#lx(x%hhu)", mask_instr_rs2(instr), stype_imm(instr), mask_instr_rs1(instr));
}

EXEC_DEF(sw) {
	STORE_WORD(READ_REG(mask_instr_rs1(instr)) + stype_imm(instr), READ_REG(mask_instr_rs2(instr)));
}

// Jump and link (jal)
DISASM_DEF(jal) {
    return DISASM_FMT("jal %hhu, %#lx", mask_instr_rd(instr), jtype_imm(instr));
}

EXEC_DEF(jal) {
    WRITE_REG(mask_instr_rd(instr), GET_PC + 4);
    SET_PC(GET_PC + jtype_imm(instr));
}

// Jump and link register (jalr)
DISASM_DEF(jalr) {
    return DISASM_FMT("jalr %hhu, %hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), itype_imm(instr));
}

EXEC_DEF(jalr) {
    dword t = GET_PC;
    SET_PC((READ_REG(mask_instr_rs1(instr)) + itype_imm(instr)) & ~1);
    WRITE_REG(mask_instr_rd(instr), t);    
}

// Branch if equal (beq)
DISASM_DEF(beq) {
    return DISASM_FMT("beq %hhu, %hhu, %#lx", mask_instr_rs1(instr), mask_instr_rs2(instr), btype_imm(instr));
}

EXEC_DEF(beq) {
    if (READ_REG(mask_instr_rs1(instr)) == READ_REG(mask_instr_rs2(instr))) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// Branch if not equal (bne)
DISASM_DEF(bne) {
    return DISASM_FMT("bne %hhu, %hhu, %#lx", mask_instr_rs1(instr), mask_instr_rs2(instr), btype_imm(instr));
}

EXEC_DEF(bne) {
    if (READ_REG(mask_instr_rs1(instr)) != READ_REG(mask_instr_rs2(instr))) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// Branch if less than (blt)
DISASM_DEF(blt) {
    return DISASM_FMT("blt %hhu, %hhu, %#lx", mask_instr_rs1(instr), mask_instr_rs2(instr), btype_imm(instr));
}

EXEC_DEF(blt) {
    if (READ_REG((sdword) mask_instr_rs1(instr)) < (sdword) READ_REG(mask_instr_rs2(instr))) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// Branch if greater than or equal (bge)
DISASM_DEF(bge) {
    return DISASM_FMT("bge %hhu, %hhu, %#lx", mask_instr_rs1(instr), mask_instr_rs2(instr), btype_imm(instr));
}

EXEC_DEF(bge) {
    if (READ_REG((sdword) mask_instr_rs1(instr)) >= (sdword) READ_REG(mask_instr_rs2(instr))) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// Unsigned branch if less than (bltu)
DISASM_DEF(bltu) {
    return DISASM_FMT("bltu %hhu, %hhu, %#lx", mask_instr_rs1(instr), mask_instr_rs2(instr), btype_imm(instr));
}

EXEC_DEF(bltu) {
    if (READ_REG((mask_instr_rs1(instr)) < READ_REG(mask_instr_rs2(instr)))) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// Unsigned branch if greater than or equal (bgeu)
DISASM_DEF(bgeu) {
    return DISASM_FMT("bgeu %hhu, %hhu, %#lx", mask_instr_rs1(instr), mask_instr_rs2(instr), btype_imm(instr));
}

EXEC_DEF(bgeu) {
    if (READ_REG(mask_instr_rs1(instr)) >= READ_REG(mask_instr_rs2(instr))) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// ---------- Wide RV64I Instructions ----------

// Add word immediate (addiw)
DISASM_DEF(addiw) {
    return DISASM_FMT("addiw %hhu, %hhu, %#lx", mask_instr_rd(instr), mask_instr_rs1(instr), itype_imm(instr));
}

EXEC_DEF(addiw) {
    WRITE_REG(mask_instr_rd(instr), (((sdword) READ_REG(mask_instr_rs1(instr)) + itype_imm(instr)) << 32 ) >> 32);
}

// TODO: slliw
// TODO: srliw
// TODO: sraiw
// TODO: addw
// TODO: subw
// TODO: sllw
// TODO: srlw
// TODO: sraw
// TODO: lwu

// Load dword (ld)
DISASM_DEF(ld) {
    return DISASM_FMT("ld %hhu, %#lx(%hhu)", mask_instr_rd(instr), itype_imm(instr), mask_instr_rs1(instr));
}

EXEC_DEF(ld) {
    WRITE_REG(mask_instr_rd(instr), LOAD_DWORD(READ_REG(mask_instr_rs1(instr) + itype_imm(instr))));
}

// Store dword (sd)
DISASM_DEF(sd) {
    return DISASM_FMT("sd %hhu, %#lx(%hhu)", mask_instr_rs2(instr), stype_imm(instr), mask_instr_rs1(instr));
}

EXEC_DEF(sd) {
    STORE_DWORD(READ_REG(mask_instr_rs1(instr)) + stype_imm(instr), READ_REG(mask_instr_rs2(instr)));
}

// Array of all implemented rv64i instruction types
riscv_instr_t const rv64i_instructions[] = {
    // Load upper immediate (lui)
	{
		.mask =    INSTR_OPCODE,
		.required_bits = OPCODE(0110111),
		INSTR_LINKS(lui)
	},

	// Add immediate (addi)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(000),
		INSTR_LINKS(addi)
	},

	// Add immediate (xori)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(100),
		INSTR_LINKS(addi)
	},

	// Or immediate (ori)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(110),
		INSTR_LINKS(ori)
	},

	// And immediate (andi)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(111),
		INSTR_LINKS(andi)
	},

	// Logical left shift by immediate (slli)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | BITSMASK(31, 26), // Only the top 6 bits of FUNCT7
		.required_bits = OPCODE(0010011) | FUNCT3(001) | FUNCT7(0000000),
		INSTR_LINKS(slli)
	},

	// Logical right shift by immediate (srli)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | BITSMASK(31, 26), // Only the top 6 bits of FUNCT7
		.required_bits = OPCODE(0010011) | FUNCT3(101) | FUNCT7(0000000),
		INSTR_LINKS(srli)
	},

	// Arithmetic right shift by immediate (srai)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | BITSMASK(31, 26), // Only the top 6 bits of FUNCT7
		.required_bits = OPCODE(0010011) | FUNCT3(101) | FUNCT7(0100000),
		INSTR_LINKS(srai)
	},

	// 64-bit addition (add)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | INSTR_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(000) |  FUNCT7(0000000),
		INSTR_LINKS(add)
	},

	// 64-bit subtraction (sub)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | INSTR_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(000) |  FUNCT7(0100000),
		INSTR_LINKS(sub)
	},

	// Logical left shift (sll)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | INSTR_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(001) |  FUNCT7(0000000),
		INSTR_LINKS(sub)
	},

	// Logical right shift (srl)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | INSTR_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(101) |  FUNCT7(0000000),
		INSTR_LINKS(srl)
	},

	// Arithmetic right shift (sra)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | INSTR_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(101) |  FUNCT7(0100000),
		INSTR_LINKS(sra)
	},

	// EBREAK
	{
		.mask =    INSTR_OPCODE |    INSTR_RD |  INSTR_FUNCT3 | INSTR_RS1 |  INSTR_RS2 |  INSTR_FUNCT7,
		.required_bits = OPCODE(1110011) | RD(00000) | FUNCT3(000) |  RS1(00000) | RS2(00001) | FUNCT7(0000000),
		INSTR_LINKS(ebreak)
	},

	// Store 32-bit (sw)
	{
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
		.required_bits = OPCODE(0100011) | FUNCT3(010),
		INSTR_LINKS(sw)
	},

    // Jump and link (jal)
    {
        .mask =    INSTR_OPCODE,
        .required_bits = OPCODE(1101111),
        INSTR_LINKS(jal)
    },

    // Jump and link register (jalr)
    {
        .mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100111) | FUNCT3(000),
        INSTR_LINKS(jalr)
    },

    // Branch if equal (beq)
    {
        .mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(000),
        INSTR_LINKS(beq)
    },

    // Branch if not equal (bne)
    {
        .mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(001),
        INSTR_LINKS(bne)
    },

    // Branch if less than (blt)
    {
        .mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(100),
        INSTR_LINKS(blt)
    },

    // Branch if greater than or equal (bge)
    {
        .mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(101),
        INSTR_LINKS(bge)
    },

    // Branch if less than (bltu)
    {
        .mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(110),
        INSTR_LINKS(bltu)
    },

    // Branch if greater than or equal (bgeu)
    {
        .mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(111),
        INSTR_LINKS(bgeu)
    },

    // Add word immediate (addiw)
    {
        .mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(0011011) | FUNCT3(000),
        INSTR_LINKS(addiw)
    },

    // Load dword (ld)
    {
        .mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(0000000) | FUNCT3(011),
        INSTR_LINKS(ld)
    },

    // Store dword (sd)
    {
        .mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(0100011) | FUNCT3(011),
        INSTR_LINKS(sd)
    },
};

// Number of implemented rv64i instructions
const size_t rv64i_size = sizeof(rv64i_instructions) / sizeof(riscv_instr_t);

#endif