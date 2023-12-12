# CpS 310 ArmSim Project: Prototype Test 2
# (c) 2016, Bob Jones University
#------------------------------------------
# Tests ROR shift mode as well as data
# processing instructions besides MOV

.global _start
.text
_start:
  addi x1, x0, 724
  li x2, 0xa1000000 

  addi x4, x0, -1
  addi x5, x0, 4
  addi x6, x5, 3
  addi x6, x5, -3
  
  andi x3, x1, 0xFF
  ori x3, x1, 0x12
  xori x3, x1, 732

  addi x3, x0, 2
  mul x6, x2, x3
  
  ebreak 
