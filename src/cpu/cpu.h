#ifndef CPU_H
#define CPU_H

#include "cpu/rv32i.h"

extern CPU_BITS cpu_pc;
extern bool cpu_running;

CPU_BITS rreg(u8 i);
void wreg(u8 i, CPU_BITS val);

#endif
