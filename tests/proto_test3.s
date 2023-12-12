# CpS 310 ArmSim Project: Prototype Test 3
# (c) 2016, Bob Jones University
#------------------------------------------
# Tests register-shifted-by-register modes

.global _start
.text
_start:
  addi x1, x0, 0xFF
  addi x2, x0, 4
  sll x3, x1, x2
  srl x4, x1, x2
  li x5, 0xF00000000000000F
  sra x6, x5, x2
  ebreak 
