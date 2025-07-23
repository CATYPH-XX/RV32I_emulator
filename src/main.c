#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "deft.h"
#include "cpu/rv32i.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

HANDLE hConsole;
CHAR_INFO console_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
SMALL_RECT window_size = {0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1};

u8 VGA[SCREEN_WIDTH * SCREEN_HEIGHT * 2];

// Инициализация консоли
void console_init() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleWindowInfo(hConsole, TRUE, &window_size);
    
    COORD buffer_size = {SCREEN_WIDTH, SCREEN_HEIGHT};
    SetConsoleScreenBufferSize(hConsole, buffer_size);
    
    CONSOLE_CURSOR_INFO cursor_info = {1, FALSE};
    SetConsoleCursorInfo(hConsole, &cursor_info);
}

// Обновление экрана
void console_render() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int offset = (y * SCREEN_WIDTH + x) * 2;
            console_buffer[y * SCREEN_WIDTH + x].Char.AsciiChar = VGA[offset];
            
            // Преобразование VGA-атрибутов в консольные
            uint8_t attr = VGA[offset + 1];
            uint8_t fg = attr & 0x0F;
            uint8_t bg = (attr >> 4) & 0x0F;
            
            // Простое преобразование цветов (может потребоваться доработка)
            console_buffer[y * SCREEN_WIDTH + x].Attributes = (bg << 4) | fg;
        }
    }
    
    WriteConsoleOutputA(hConsole, console_buffer, 
                       (COORD){SCREEN_WIDTH, SCREEN_HEIGHT}, 
                       (COORD){0, 0}, &window_size);
}

CPU_BITS vga_read(CPU_BITS addr, u8 SEL) {
    bool b1 = SEL & 0b0001;
    bool b2 = SEL & 0b0010;
    bool b3 = SEL & 0b0100;
    bool b4 = SEL & 0b1000;

    CPU_BITS ret = 0;

    if (b1) {
        ret = (CPU_BITS)VGA[addr];
    }

    if (b2) {
        ret = ret | (CPU_BITS)(VGA[addr + 1] << 8);
    }

    if (b3) {
        ret = ret | (CPU_BITS)(VGA[addr + 2] << 16);
    }

    if (b4) {
        ret = ret | (CPU_BITS)(VGA[addr + 3] << 24);
    }

    return ret;
}

void vga_write(CPU_BITS addr, CPU_BITS val, u8 SEL) {
    bool b1 = SEL & 0b0001;
    bool b2 = SEL & 0b0010;
    bool b3 = SEL & 0b0100;
    bool b4 = SEL & 0b1000;

    CPU_BITS write = 0;

    if (b1) {
        VGA[addr] = val & 0xFF;
    }

    if (b2) {
        VGA[addr+1] = val & 0xFF00;
    }

    if (b3) {
        VGA[addr+2] = val & 0xFF0000;
    }

    if (b4) {
        VGA[addr+3] = val & 0xFF000000;
    }

    console_render();
}


int main() {
    console_init();

    cpu_mem_attach_dev(0x80100000, 4000, &vga_write, &vga_read);

    void* RAM = malloc(1024 * 1024);

    cpu_mem_attach_ram(RAM, 0x80000000, 1024 * 1024);

   

    system("pause");
    return 0;
}
