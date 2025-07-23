// основной файл-интерфейс, содержащий все функции для работы с процессором

#ifndef RV32I_H
#define RV32I_H

#include "deft.h"

// разрядность процессора - размер машинного слова, указателя, одной инструкции, PC и регистров x0...x31
#define CPU_BITS u32

extern CPU_BITS CPU_START_ADDR;

void cpu_run();

void cpu_mem_attach_ram(
    void* ptr,
    CPU_BITS addr,
    CPU_BITS size
);

void cpu_mem_attach_dev(
    CPU_BITS addr,
    CPU_BITS size,

    void (*on_write)(CPU_BITS addr, CPU_BITS value, u8 SEL),
    CPU_BITS (*on_read)(CPU_BITS addr, u8 SEL)
);

#endif
