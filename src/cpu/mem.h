#ifndef MEM_H
#define MEM_H

#include "deft.h"
#include "cpu/rv32i.h"

void mem_write_8(CPU_BITS addr, u8 val);
void mem_write_16(CPU_BITS addr, u16 val);
void mem_write_32(CPU_BITS addr, u32 val);

u8 mem_read_8(CPU_BITS addr);
u16 mem_read_16(CPU_BITS addr);
u32 mem_read_32(CPU_BITS addr);

#endif
