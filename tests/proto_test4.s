# CpS 310 ArmSim Project: Prototype Test 4
# (c) 2016, Bob Jones University
#------------------------------------------
# Baseline/minimal load/store support

.global _start
.text
_start:
  li x1, 0xfb0 
  li x2, 0x5000
  li x3, 0x3000 
  addi x4, x0, 8
  
  
  sw x1, 0(x2)
  sw x3, -4(x2)     # 4ffc
  
  lw x5, 0(x2)
  lw x6, -4(x2)     # 4ffc
  
  ebreak
