#ifndef SIM_RV64I_INSTRUCTIONS
#define SIM_RV64I_INSTRUCTIONS

// The RV64I Base Integer Instruction Set

// ---------- Dissasembly and Execution Functions ----------

// Load upper immediate (lui)
DISASM_DEF(lui) {
	sword imm = utype_imm(instr);
	return DISASM_FMT(imm < 0 ? "lui x%hhu, %d" : "lui x%hhu, %#x", GET_RD, imm);
}

EXEC_DEF(lui) {
	WRITE_REG(GET_RD, utype_imm(instr));
}

// TODO: auipc

// Add immediate (addi)
DISASM_DEF(addi) {
	sword imm = itype_imm(instr);
	return DISASM_FMT(imm < 0 ? "addi x%hhu, x%hhu, %d" : "addi x%hhu, x%hhu, %#x", GET_RD, GET_RS1, imm);
}

EXEC_DEF(addi) {
	WRITE_REG(GET_RD, READ_REG(GET_RS1) + itype_imm(instr));
}

// TODO: slti
// TODO: sltiu

// XOR immediate (xori)
DISASM_DEF(xori) {
	sword imm = itype_imm(instr);
	return DISASM_FMT(imm < 0 ? "xori x%hhu, x%hhu, %d" : "xori x%hhu, x%hhu, %#x", GET_RD, GET_RS1, imm);
}

EXEC_DEF(xori) {
	WRITE_REG(GET_RD, READ_REG(GET_RS1) ^ itype_imm(instr));
}

// OR immediate (ori)
DISASM_DEF(ori) {
	sword imm = itype_imm(instr);
	return DISASM_FMT(imm < 0 ? "ori x%hhu, x%hhu, %d" : "ori x%hhu, x%hhu, %#x", GET_RD, GET_RS1, imm);
}

EXEC_DEF(ori) {
	WRITE_REG(GET_RD, READ_REG(GET_RS1) | itype_imm(instr));
}

// AND immediate (andi)
DISASM_DEF(andi) {
	sword imm = itype_imm(instr);
	return DISASM_FMT(imm < 0 ? "andi x%hhu, x%hhu, %d" : "andi x%hhu, x%hhu, %#x", GET_RD, GET_RS1, imm);
}

EXEC_DEF(andi) {
	WRITE_REG(GET_RD, READ_REG(GET_RS1) & itype_imm(instr));
}

// Immediate logical shift left (slli)
DISASM_DEF(slli) {
	dword imm = (BITSMASK(25, 20) & instr) >> 20;
	return DISASM_FMT("slli x%hhu, x%hhu, %#lx", GET_RD, GET_RS1, imm);
}

EXEC_DEF(slli) {
	WRITE_REG(GET_RD, READ_REG(GET_RS1) << itype_imm(instr));
}

// Immediate logical right shift (srli)
DISASM_DEF(srli) {
	dword imm = (BITSMASK(25, 20) & instr) >> 20;
	return DISASM_FMT("srli x%hhu, x%hhu, %#lx", GET_RD, GET_RS1, imm);
}

EXEC_DEF(srli) {
	dword imm = (BITSMASK(25, 20) & instr) >> 20;
	WRITE_REG(GET_RD, READ_REG(GET_RS1) >> imm);
}

// Immediate arithmetic right shift (srai)
DISASM_DEF(srai) {
	dword imm = (BITSMASK(25, 20) & instr) >> 20;
	return DISASM_FMT("srai x%hhu, x%hhu, %#lx", GET_RD, GET_RS1, imm);
}

EXEC_DEF(srai) {
	dword imm = (BITSMASK(25, 20) & instr) >> 20;
	WRITE_REG(GET_RD, ((sdword) READ_REG(GET_RS1)) >> imm);
}

// 64-bit addition (add)
DISASM_DEF(add) {
	return DISASM_FMT("add x%hhu, x%hhu, x%hhu", GET_RD, GET_RS1, GET_RS2);
}

EXEC_DEF(add) {
	WRITE_REG(GET_RD, READ_REG(GET_RS1) + READ_REG(GET_RS2));
}

// 64-bit subtraction (sub)
DISASM_DEF(sub) {
	return DISASM_FMT("sub x%hhu, x%hhu, x%hhu", GET_RD, GET_RS1, GET_RS2);
}

EXEC_DEF(sub) {
	WRITE_REG(GET_RD, READ_REG(GET_RS1) - READ_REG(GET_RS2));
}

// Logical left shift (sll)
DISASM_DEF(sll) {
	return DISASM_FMT("sll x%hhu, x%hhu, x%hhu", GET_RD, GET_RS1, GET_RS2);
}

EXEC_DEF(sll) {
	WRITE_REG(GET_RD, READ_REG(GET_RS1) << READ_REG(GET_RS2));
}

// TODO: slt
// TODO: sltu
// TODO: xor

// Logical right shift (srl)
DISASM_DEF(srl) {
	return DISASM_FMT("srl x%hhu, x%hhu, x%hhu", GET_RD, GET_RS1, GET_RS2);
}

EXEC_DEF(srl) {
	WRITE_REG(GET_RD, READ_REG(GET_RS1) >> READ_REG(GET_RS2));
}

// Arithmetic right shift (sra)
DISASM_DEF(sra) {
	return DISASM_FMT("sra x%hhu, x%hhu, x%hhu", GET_RD, GET_RS1, GET_RS2);
}

EXEC_DEF(sra) {
	WRITE_REG(GET_RD, ((sdword) READ_REG(GET_RS1)) >> READ_REG(GET_RS2));
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
#define RV64I_EBREAK (FUNCT7(0000000) | RS2(00001) | RS1(00000) | FUNCT3(000) | RD(00000) | OPCODE(1110011))
DISASM_DEF(ebreak) {
	return DISASM_FMT("ebreak");
}

EXEC_DEF(ebreak) {
	// TODO: raise breakpoint exception
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
	sword imm = itype_imm(instr);
	return DISASM_FMT(imm < 0 ? "lw x%hhu, %d(x%hhu)" : "lw x%hhu, %#x(x%hhu)", GET_RD, imm, GET_RS1);
}

EXEC_DEF(lw) {
	WRITE_REG(GET_RD, LOAD_WORD(READ_REG(GET_RS1) + itype_imm(instr)));
	*loaded=1;
}

// TODO: lbu
// TODO: lhu
// TODO: sb
// TODO: sh

// Store 32-bit (sw)
DISASM_DEF(sw) {
	sword imm = stype_imm(instr);
	return DISASM_FMT(imm < 0 ? "sw x%hhu, %d(x%hhu)" : "sw x%hhu, %#x(x%hhu)", GET_RS2, imm, GET_RS1);
}

EXEC_DEF(sw) {
	STORE_WORD(READ_REG(GET_RS1) + stype_imm(instr), READ_REG(GET_RS2));
}

// Jump and link (jal)
DISASM_DEF(jal) {
	sword imm = jtype_imm(instr);
    return DISASM_FMT(imm < 0 ? "jal %hhu, %d" : "jal %hhu, %#x", GET_RD, imm);
}

EXEC_DEF(jal) {
    WRITE_REG(GET_RD, GET_PC + 4);
    SET_PC(GET_PC + jtype_imm(instr));
}

// Jump and link register (jalr)
DISASM_DEF(jalr) {
	sword imm = itype_imm(instr);
    return DISASM_FMT(imm < 0 ? "jalr %hhu, %hhu, %d" : "jalr %hhu, %hhu, %#x", GET_RD, GET_RS1, imm);
}

EXEC_DEF(jalr) {
    dword t = GET_PC;
    SET_PC((READ_REG(GET_RS1) + itype_imm(instr)) & ~1);
    WRITE_REG(GET_RD, t);    
}

// Branch if equal (beq)
DISASM_DEF(beq) {
	sword imm = btype_imm(instr);
    return DISASM_FMT(imm < 0 ? "beq %hhu, %hhu, %d" : "beq %hhu, %hhu, %#x", GET_RS1, GET_RS2, imm);
}

EXEC_DEF(beq) {
    if (READ_REG(GET_RS1) == READ_REG(GET_RS2)) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// Branch if not equal (bne)
DISASM_DEF(bne) {
	sword imm = btype_imm(instr);
    return DISASM_FMT(imm < 0 ? "bne %hhu, %hhu, %d" : "bne %hhu, %hhu, %#x", GET_RS1, GET_RS2, imm);
}

EXEC_DEF(bne) {
    if (READ_REG(GET_RS1) != READ_REG(GET_RS2)) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// Branch if less than (blt)
DISASM_DEF(blt) {
	sword imm = btype_imm(instr);
    return DISASM_FMT(imm < 0 ? "blt %hhu, %hhu, %d" : "blt %hhu, %hhu, %#x", GET_RS1, GET_RS2, imm);
}

EXEC_DEF(blt) {
    if ((sdword) READ_REG( GET_RS1) < (sdword) READ_REG(GET_RS2)) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// Branch if greater than or equal (bge)
DISASM_DEF(bge) {
	sword imm = btype_imm(instr);
    return DISASM_FMT(imm < 0 ? "bge %hhu, %hhu, %d" : "bge %hhu, %hhu, %#x", GET_RS1, GET_RS2, imm);
}

EXEC_DEF(bge) {
    if ((sdword) READ_REG(GET_RS1) >= (sdword) READ_REG(GET_RS2)) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// Unsigned branch if less than (bltu)
DISASM_DEF(bltu) {
	sword imm = btype_imm(instr);
    return DISASM_FMT(imm < 0 ? "bltu %hhu, %hhu, %d" : "bltu %hhu, %hhu, %#x", GET_RS1, GET_RS2, imm);
}

EXEC_DEF(bltu) {
    if (READ_REG(GET_RS1) < READ_REG(GET_RS2)) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// Unsigned branch if greater than or equal (bgeu)
DISASM_DEF(bgeu) {
	sword imm = btype_imm(instr);
    return DISASM_FMT(imm < 0 ? "bgeu %hhu, %hhu, %d" : "bgeu %hhu, %hhu, %#x", GET_RS1, GET_RS2, imm);
}

EXEC_DEF(bgeu) {
    if (READ_REG(GET_RS1) >= READ_REG(GET_RS2)) {
        SET_PC(GET_PC + btype_imm(instr));
    }
}

// ---------- Word RV64I Instructions ----------

// Add word immediate (addiw)
DISASM_DEF(addiw) {
	sword imm = itype_imm(instr);
    return DISASM_FMT(imm < 0 ? "addiw %hhu, %hhu, %d" : "addiw %hhu, %hhu, %#x", GET_RD, GET_RS1, imm);
}

EXEC_DEF(addiw) {
    WRITE_REG(GET_RD, (((sdword) READ_REG(GET_RS1) + itype_imm(instr)) << 32 ) >> 32);
}

// TODO: slliw
// TODO: srliw
// TODO: sraiw

// Add word
DISASM_DEF(addw) {
	return DISASM_FMT("addw x%hhu, x%hhu, x%hhu", GET_RD, GET_RS1, GET_RS2);
}

EXEC_DEF(addw) {
	sdword sum = (sdword) READ_REG(GET_RS1) + (sdword) READ_REG(GET_RS2);
	WRITE_REG(GET_RD, (sum << 32) >> 32);
}

// TODO: subw
// TODO: sllw
// TODO: srlw
// TODO: sraw
// TODO: lwu

// Load dword (ld)
DISASM_DEF(ld) {
	sword imm = itype_imm(instr);
    return DISASM_FMT(imm < 0 ? "ld %hhu, %d(%hhu)" : "ld %hhu, %#x(%hhu)", GET_RD, imm, GET_RS1);
}

EXEC_DEF(ld) {
    WRITE_REG(GET_RD, LOAD_DWORD(READ_REG(GET_RS1) + itype_imm(instr)));
	*loaded=1;
}

// Store dword (sd)
DISASM_DEF(sd) {
	sword imm = stype_imm(instr);
    return DISASM_FMT(imm < 0 ? "sd %hhu, %d(%hhu)" : "sd %hhu, %#x(%hhu)", GET_RS2, imm, GET_RS1);
}

EXEC_DEF(sd) {
    STORE_DWORD(READ_REG(GET_RS1) + stype_imm(instr), READ_REG(GET_RS2));
}

// Array of all implemented rv64i instruction types
riscv_instr_t rv64i_instructions[] = {
    // Load upper immediate (lui)
	{
		.name = "lui",
		.mask =    INSTR_OPCODE,
		.required_bits = OPCODE(0110111),
		INSTR_LINKS(lui)
	},

	// Add immediate (addi)
	{
		.name = "addi",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(000),
		INSTR_LINKS(addi)
	},

	// Add immediate (xori)
	{
		.name = "xori",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(100),
		INSTR_LINKS(xori)
	},

	// Or immediate (ori)
	{
		.name = "ori",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(110),
		INSTR_LINKS(ori)
	},

	// And immediate (andi)
	{
		.name = "andi",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
		.required_bits = OPCODE(0010011) | FUNCT3(111),
		INSTR_LINKS(andi)
	},

	// Logical left shift by immediate (slli)
	{
		.name = "slli",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | BITSMASK(31, 26), // Only the top 6 bits of FUNCT7
		.required_bits = OPCODE(0010011) | FUNCT3(001) | FUNCT7(0000000),
		INSTR_LINKS(slli)
	},

	// Logical right shift by immediate (srli)
	{
		.name = "srli",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | BITSMASK(31, 26), // Only the top 6 bits of FUNCT7
		.required_bits = OPCODE(0010011) | FUNCT3(101) | FUNCT7(0000000),
		INSTR_LINKS(srli)
	},

	// Arithmetic right shift by immediate (srai)
	{
		.name = "srai",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | BITSMASK(31, 26), // Only the top 6 bits of FUNCT7
		.required_bits = OPCODE(0010011) | FUNCT3(101) | FUNCT7(0100000),
		INSTR_LINKS(srai)
	},

	// 64-bit addition (add)
	{
		.name = "add",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | INSTR_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(000) |  FUNCT7(0000000),
		INSTR_LINKS(add)
	},

	// 64-bit subtraction (sub)
	{
		.name = "sub",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | INSTR_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(000) |  FUNCT7(0100000),
		INSTR_LINKS(sub)
	},

	// Logical left shift (sll)
	{
		.name = "sll",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | INSTR_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(001) |  FUNCT7(0000000),
		INSTR_LINKS(sll)
	},

	// Logical right shift (srl)
	{
		.name = "srl",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | INSTR_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(101) |  FUNCT7(0000000),
		INSTR_LINKS(srl)
	},

	// Arithmetic right shift (sra)
	{
		.name = "sra",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3 | INSTR_FUNCT7,
		.required_bits = OPCODE(0110011) | FUNCT3(101) |  FUNCT7(0100000),
		INSTR_LINKS(sra)
	},

	// EBREAK
	{
		.name = "ebreak",
		.mask =    INSTR_OPCODE |    INSTR_RD |  INSTR_FUNCT3 | INSTR_RS1 |  INSTR_RS2 |  INSTR_FUNCT7,
		.required_bits = OPCODE(1110011) | RD(00000) | FUNCT3(000) |  RS1(00000) | RS2(00001) | FUNCT7(0000000),
		INSTR_LINKS(ebreak)
	},

	// Load 32 bit (lw)
	{
		.name = "lw",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
		.required_bits = OPCODE(0000011) | FUNCT3(010),
		INSTR_LINKS(lw)
	},

	// Store 32-bit (sw)
	{
		.name = "sw",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
		.required_bits = OPCODE(0100011) | FUNCT3(010),
		INSTR_LINKS(sw)
	},

    // Jump and link (jal)
	{
		.name = "jal",
		.mask =    INSTR_OPCODE,
        .required_bits = OPCODE(1101111),
        INSTR_LINKS(jal)
    },

    // Jump and link register (jalr)
	{
		.name = "jalr",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100111) | FUNCT3(000),
        INSTR_LINKS(jalr)
    },

    // Branch if equal (beq)
	{
		.name = "beq",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(000),
        INSTR_LINKS(beq)
    },

    // Branch if not equal (bne)
	{
		.name = "bne",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(001),
        INSTR_LINKS(bne)
    },

    // Branch if less than (blt)
	{
		.name = "blt",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(100),
        INSTR_LINKS(blt)
    },

    // Branch if greater than or equal (bge)
	{
		.name = "bge",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(101),
        INSTR_LINKS(bge)
    },

    // Branch if less than (bltu)
	{
		.name = "bltu",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(110),
        INSTR_LINKS(bltu)
    },

    // Branch if greater than or equal (bgeu)
	{
		.name = "bgeu",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(1100011) | FUNCT3(111),
        INSTR_LINKS(bgeu)
    },

    // Add word immediate (addiw)
	{
		.name = "addiw",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(0011011) | FUNCT3(000),
        INSTR_LINKS(addiw)
    },

	// Add word (addw)
	{
		.name = "addw",
		.mask = INSTR_OPCODE |  INSTR_FUNCT3 | INSTR_FUNCT7,
		.required_bits = OPCODE(0111011) | FUNCT3(000) | FUNCT7(0000000),
		INSTR_LINKS(addw)
	},

    // Load dword (ld)
	{
		.name = "ld",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(0000011) | FUNCT3(011),
        INSTR_LINKS(ld)
    },

    // Store dword (sd)
	{
		.name = "sd",
		.mask =    INSTR_OPCODE |    INSTR_FUNCT3,
        .required_bits = OPCODE(0100011) | FUNCT3(011),
        INSTR_LINKS(sd)
    },
};

// Number of implemented rv64i instructions
const size_t rv64i_size = sizeof(rv64i_instructions) / sizeof(riscv_instr_t);

#endif