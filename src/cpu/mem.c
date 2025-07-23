#include "cpu/mem.h"
#include "deft.h"
#include "cpu/rv32i.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    CPU_BITS start_addr;
    CPU_BITS end_addr;

    void (*on_write)(CPU_BITS addr, CPU_BITS value, u8 SEL);
    CPU_BITS (*on_read)(CPU_BITS addr, u8 SEL);
} Device;

typedef struct {
    u8* ptr;
    CPU_BITS start_addr;
    CPU_BITS end_addr;
} RAM;

static Device devices[32]; // не думаю, что будет больше 32 устройств
static int devices_c = 0;

static RAM rams[32];
static int rams_c = 0;

void cpu_mem_attach_dev(
    CPU_BITS addr,
    CPU_BITS size,

    void (*on_write)(CPU_BITS addr, CPU_BITS value, u8 SEL),
    CPU_BITS (*on_read)(CPU_BITS addr, u8 SEL)
) {
    devices[devices_c].start_addr = addr;
    devices[devices_c].end_addr = addr + size;
    devices[devices_c].on_write = on_write;
    devices[devices_c].on_read = on_read;
    devices_c++;
}

void cpu_mem_attach_ram(
    void* ptr,
    CPU_BITS addr,
    CPU_BITS size
) {
    if ((uint)ptr % 4 != 0) {
        printf("error: ram address isn't alligned for 4");
        exit(1);
    }

    rams[rams_c].ptr = (u8*)ptr;
    rams[rams_c].start_addr = addr;
    rams[rams_c].end_addr = addr + size;
    rams_c++;
}

void mem_write_8(CPU_BITS addr, u8 val) {
    for (int i = 0; i < rams_c; i++) {
        if (addr >= rams[i].start_addr && addr < rams[i].end_addr) {
            rams[i].ptr[addr - rams[i].start_addr] = val; // себе будущему: здесь сначала берется нужный ram из списка rams, затем оттуда берется элемент, индекс которого вычисляется по формуле (адрес - адрес начала)
            return;
        }
    }

    for (int i = 0; i < devices_c; i++) {
        if (addr >= devices[i].start_addr && addr < devices[i].end_addr) {
            devices[i].on_write(addr, (u32)val, 0b00000001);
            return;
        }
    }

    printf("attempt to write 8-bit value 0x%02X to unmapped address 0x%08X", val, addr);
}

void mem_write_16(CPU_BITS addr, u16 val) {
    if (addr % 2 != 0) {
        printf("attempt to write 16-bit value 0x%04X to unaligned address 0x%08X", val, addr);
        return;
    }
    
    for (int i = 0; i < rams_c; i++) {
        if (addr >= rams[i].start_addr && addr < rams[i].end_addr) {
            rams[i].ptr[addr - rams[i].start_addr] = val & 0xFF;
            rams[i].ptr[addr - rams[i].start_addr + 1] = (val >> 8) & 0xFF;
            return;
        }
    }

    for (int i = 0; i < devices_c; i++) {
        if (addr >= devices[i].start_addr && addr < devices[i].end_addr) {
            devices[i].on_write(addr, (u32)val, 0b00000011);
            return;
        }
    }

    printf("attempt to write 16-bit value 0x%04X to unmapped address 0x%08X", val, addr);
}

void mem_write_32(CPU_BITS addr, u32 val) {
    if (addr % 4 != 0) {
        printf("attempt to write 32-bit value 0x%08X to unaligned address 0x%08X", val, addr);
        return;
    }

    for (int i = 0; i < rams_c; i++) {
        if (addr >= rams[i].start_addr && addr < rams[i].end_addr) {
            rams[i].ptr[addr - rams[i].start_addr] = val & 0xFF;
            rams[i].ptr[addr - rams[i].start_addr + 1] = (val >> 8)  & 0xFF;
            rams[i].ptr[addr - rams[i].start_addr + 2] = (val >> 16) & 0xFF;
            rams[i].ptr[addr - rams[i].start_addr + 3] = (val >> 24) & 0xFF;
            return;
        }
    }

    for (int i = 0; i < devices_c; i++) {
        if (addr >= devices[i].start_addr && addr < devices[i].end_addr) {
            devices[i].on_write(addr, val, 0b00001111);
            return;
        }
    }

    printf("attempt to write 32-bit value 0x%08X to unmapped address 0x%08X", val, addr);
}

u8 mem_read_8(CPU_BITS addr) {
    for (int i = 0; i < rams_c; i++) {
        if (addr >= rams[i].start_addr && addr < rams[i].end_addr) {
            return rams[i].ptr[addr - rams[i].start_addr];
        }
    }

    for (int i = 0; i < devices_c; i++) {
        if (addr >= devices[i].start_addr && addr < devices[i].end_addr) {
            return devices[i].on_read(addr, 0b00000001);
        }
    }

    printf("attempt to read 8-bit value from unmapped address 0x%08X", addr);
    return 0;
}

u16 mem_read_16(CPU_BITS addr) {
    if (addr % 2 != 0) {
        printf("attempt to read 16-bit value from unaligned address 0x%08X", addr);
        return 0;
    }

    for (int i = 0; i < rams_c; i++) {
        if (addr >= rams[i].start_addr && addr < rams[i].end_addr) {
            u32 p1 = (u32)(rams[i].ptr[addr - rams[i].start_addr]);
            u32 p2 = (u32)(rams[i].ptr[addr - rams[i].start_addr + 1]) << 8;
            return p1 | p2;
        }
    }

    for (int i = 0; i < devices_c; i++) {
        if (addr >= devices[i].start_addr && addr < devices[i].end_addr) {
            return devices[i].on_read(addr, 0b00000011);
        }
    }

    printf("attempt to read 16-bit value from unmapped address 0x%08X", addr);
    return 0;
}

u32 mem_read_32(CPU_BITS addr){
    if (addr % 4 != 0) {
        printf("attempt to read 32-bit value from unaligned address 0x%08X", addr);
        return 0;
    }

    for (int i = 0; i < rams_c; i++) {
        if (addr >= rams[i].start_addr && addr < rams[i].end_addr) {
            u32 p1 = (u32)(rams[i].ptr[addr - rams[i].start_addr]);
            u32 p2 = (u32)(rams[i].ptr[addr - rams[i].start_addr + 1]) << 8;
            u32 p3 = (u32)(rams[i].ptr[addr - rams[i].start_addr + 2]) << 16;
            u32 p4 = (u32)(rams[i].ptr[addr - rams[i].start_addr + 3]) << 24;
            return p1 | p2 | p3 | p4;
        }
    }

    for (int i = 0; i < devices_c; i++) {
        if (addr >= devices[i].start_addr && addr < devices[i].end_addr) {
            return devices[i].on_read(addr, 0b00001111);
        }
    }

    printf("attempt to read 32-bit value from unmapped address 0x%08X", addr);
    return 0;
}
