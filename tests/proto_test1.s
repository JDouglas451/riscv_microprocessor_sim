# CpS 310 ArmSim Project: Prototype Test 1
# (c) 2016, Bob Jones University
#------------------------------------------
# Tests MOV instruction with basic addressing modes

.global _start
.text
_start:
  addi x1, x0, 724
  li x2, 0xa100000000000000
  add x3, x0, x0
  srai x3, x2, 2
  srli x3, x2, 2
  slli x3, x2, 1
  ebreak
