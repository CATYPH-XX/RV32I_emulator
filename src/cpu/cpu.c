#include "cpu/cpu.h"
#include "cpu/rv32i.h"
#include "cpu/decode.h"
#include "cpu/mem.h"
#include "deft.h"

#include <stdlib.h>
#include <stdio.h>

CPU_BITS CPU_START_ADDR = 0x0;
CPU_BITS cpu_pc;
bool cpu_running;

void cpu_err() {
    printf("RISC CPU Error");
    exit(1);
}

CPU_BITS regs[31];

CPU_BITS rreg(u8 i) {
    if (i == 0) return 0;
    else if (i > 31 || i < 1) cpu_err();
    return regs[i-1];
}

void wreg(u8 i, CPU_BITS val) {
    if (i == 0) return;
    else if (i > 31 || i < 1) cpu_err();
    regs[i-1] = val;
}

void cpu_run() {
    cpu_pc = CPU_START_ADDR;
    cpu_running = true;

    while(cpu_running) {
        decode_and_execute(mem_read_32(cpu_pc));
        
        cpu_pc++;
    }
}
