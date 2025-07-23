#include "cpu/decode.h"
#include "cpu/cpu.h"
#include "cpu/mem.h"
#include "cpu/rv32i.h"
#include "deft.h"

typedef enum {
    OP_LUI,
    OP_AUIPC,
    OP_JAL,
    OP_JALR,
    OP_BEQ,
    OP_BNE,
    OP_BLT,
    OP_BGE,
    OP_BLTU,
    OP_BGEU,
    OP_LB,
    OP_LH,
    OP_LW,
    OP_LBU,
    OP_LHU,
    OP_SB,
    OP_SH,
    OP_SW,
    OP_ADDI,
    OP_SLTI,
    OP_SLTIU,
    OP_XORI,
    OP_ORI,
    OP_ANDI,
    OP_SLLI,
    OP_SRLI,
    OP_SRAI,
    OP_ADD,
    OP_SUB,
    OP_SLL,
    OP_SLT,
    OP_SLTU,
    OP_XOR,
    OP_SRL,
    OP_SRA,
    OP_OR,
    OP_AND,
    OP_FENCE,
    OP_FENCE_TSO,
    OP_PAUSE,
    OP_ECALL,
    OP_EBREAK,
    OP_UNKNOWN
} Opcode;

typedef struct {
    Opcode op;
    u32 rd;
    u32 rs1;
    u32 rs2;
    u32 imm;
    u32 funct3;
    u32 funct7;
} Instruction;

Instruction decode(CPU_BITS instr) {
    Instruction inst = {0};
    u32 opcode = instr & 0x7F;
    inst.funct3 = (instr >> 12) & 0x7;
    inst.rd = (instr >> 7) & 0x1F;
    inst.rs1 = (instr >> 15) & 0x1F;
    inst.rs2 = (instr >> 20) & 0x1F;
    inst.funct7 = (instr >> 25) & 0x7F;

    // Decode immediate values based on instruction type
    switch (opcode) {
        case 0x37: // LUI
            inst.op = OP_LUI;
            inst.imm = (instr & 0xFFFFF000);
            break;
            
        case 0x17: // AUIPC
            inst.op = OP_AUIPC;
            inst.imm = (instr & 0xFFFFF000);
            break;
            
        case 0x6F: // JAL
            inst.op = OP_JAL;
            inst.imm = ((instr >> 31) & 0x1) << 20 |
                      ((instr >> 21) & 0x3FF) << 1 |
                      ((instr >> 20) & 0x1) << 11 |
                      ((instr >> 12) & 0xFF) << 12;
            // Sign extend
            if (inst.imm & 0x100000) inst.imm |= 0xFFF00000;
            break;
            
        case 0x67: // JALR
            inst.op = OP_JALR;
            inst.imm = (instr >> 20) & 0xFFF;
            // Sign extend
            if (inst.imm & 0x800) inst.imm |= 0xFFFFF000;
            break;
            
        case 0x63: // B-type
            inst.imm = ((instr >> 31) & 0x1) << 12 |
                      ((instr >> 25) & 0x3F) << 5 |
                      ((instr >> 8) & 0xF) << 1 |
                      ((instr >> 7) & 0x1) << 11;
            // Sign extend
            if (inst.imm & 0x1000) inst.imm |= 0xFFFFE000;
            
            switch (inst.funct3) {
                case 0x0: inst.op = OP_BEQ; break;
                case 0x1: inst.op = OP_BNE; break;
                case 0x4: inst.op = OP_BLT; break;
                case 0x5: inst.op = OP_BGE; break;
                case 0x6: inst.op = OP_BLTU; break;
                case 0x7: inst.op = OP_BGEU; break;
                default: inst.op = OP_UNKNOWN; break;
            }
            break;
            
        case 0x03: // I-type (load)
            inst.imm = (instr >> 20) & 0xFFF;
            // Sign extend
            if (inst.imm & 0x800) inst.imm |= 0xFFFFF000;
            
            switch (inst.funct3) {
                case 0x0: inst.op = OP_LB; break;
                case 0x1: inst.op = OP_LH; break;
                case 0x2: inst.op = OP_LW; break;
                case 0x4: inst.op = OP_LBU; break;
                case 0x5: inst.op = OP_LHU; break;
                default: inst.op = OP_UNKNOWN; break;
            }
            break;
            
        case 0x23: // S-type
            inst.imm = ((instr >> 25) & 0x7F) << 5 | ((instr >> 7) & 0x1F);
            // Sign extend
            if (inst.imm & 0x800) inst.imm |= 0xFFFFF000;
            
            switch (inst.funct3) {
                case 0x0: inst.op = OP_SB; break;
                case 0x1: inst.op = OP_SH; break;
                case 0x2: inst.op = OP_SW; break;
                default: inst.op = OP_UNKNOWN; break;
            }
            break;
            
        case 0x13: // I-type (ALU)
            inst.imm = (instr >> 20) & 0xFFF;
            // Sign extend
            if (inst.imm & 0x800) inst.imm |= 0xFFFFF000;
            
            switch (inst.funct3) {
                case 0x0: inst.op = OP_ADDI; break;
                case 0x2: inst.op = OP_SLTI; break;
                case 0x3: inst.op = OP_SLTIU; break;
                case 0x4: inst.op = OP_XORI; break;
                case 0x6: inst.op = OP_ORI; break;
                case 0x7: inst.op = OP_ANDI; break;
                case 0x1: inst.op = OP_SLLI; break;
                case 0x5: 
                    if ((instr >> 30) & 0x1) inst.op = OP_SRAI;
                    else inst.op = OP_SRLI;
                    break;
                default: inst.op = OP_UNKNOWN; break;
            }
            break;
            
        case 0x33: // R-type
            switch (inst.funct3) {
                case 0x0: 
                    if ((instr >> 30) & 0x1) inst.op = OP_SUB;
                    else inst.op = OP_ADD;
                    break;
                case 0x1: inst.op = OP_SLL; break;
                case 0x2: inst.op = OP_SLT; break;
                case 0x3: inst.op = OP_SLTU; break;
                case 0x4: inst.op = OP_XOR; break;
                case 0x5: 
                    if ((instr >> 30) & 0x1) inst.op = OP_SRA;
                    else inst.op = OP_SRL;
                    break;
                case 0x6: inst.op = OP_OR; break;
                case 0x7: inst.op = OP_AND; break;
                default: inst.op = OP_UNKNOWN; break;
            }
            break;
            
        case 0x0F: // FENCE
            if (instr == 0x8330000F) inst.op = OP_FENCE_TSO;
            else if (instr == 0x0100000F) inst.op = OP_PAUSE;
            else inst.op = OP_FENCE;
            break;
            
        case 0x73: // SYSTEM
            if (inst.imm == 0) inst.op = OP_ECALL;
            else if (inst.imm == 1) inst.op = OP_EBREAK;
            else inst.op = OP_UNKNOWN;
            break;
            
        default:
            inst.op = OP_UNKNOWN;
            break;
    }
    
    return inst;
}

void execute(Instruction inst) {
    u32 val1, val2, result;
    
    switch (inst.op) {
        // Арифметические операции
        case OP_LUI:
            wreg(inst.rd, inst.imm);
            break;
            
        case OP_AUIPC:
            wreg(inst.rd, cpu_pc + inst.imm);
            break;
            
        case OP_ADDI:
            wreg(inst.rd, rreg(inst.rs1) + inst.imm);
            break;
            
        case OP_ADD:
            wreg(inst.rd, rreg(inst.rs1) + rreg(inst.rs2));
            break;
            
        case OP_SUB:
            wreg(inst.rd, rreg(inst.rs1) - rreg(inst.rs2));
            break;
            
        case OP_SLTI:
            wreg(inst.rd, (i32)rreg(inst.rs1) < (i32)inst.imm ? 1 : 0);
            break;
            
        case OP_SLTIU:
            wreg(inst.rd, rreg(inst.rs1) < (u32)inst.imm ? 1 : 0);
            break;
            
        // Логические операции
        case OP_XORI:
            wreg(inst.rd, rreg(inst.rs1) ^ inst.imm);
            break;
            
        case OP_XOR:
            wreg(inst.rd, rreg(inst.rs1) ^ rreg(inst.rs2));
            break;
            
        case OP_ORI:
            wreg(inst.rd, rreg(inst.rs1) | inst.imm);
            break;
            
        case OP_OR:
            wreg(inst.rd, rreg(inst.rs1) | rreg(inst.rs2));
            break;
            
        case OP_ANDI:
            wreg(inst.rd, rreg(inst.rs1) & inst.imm);
            break;
            
        case OP_AND:
            wreg(inst.rd, rreg(inst.rs1) & rreg(inst.rs2));
            break;
            
        // Сдвиги
        case OP_SLLI:
            wreg(inst.rd, rreg(inst.rs1) << (inst.imm & 0x1F));
            break;
            
        case OP_SRLI:
            wreg(inst.rd, rreg(inst.rs1) >> (inst.imm & 0x1F));
            break;
            
        case OP_SRAI:
            wreg(inst.rd, (i32)rreg(inst.rs1) >> (inst.imm & 0x1F));
            break;
            
        case OP_SLL:
            wreg(inst.rd, rreg(inst.rs1) << (rreg(inst.rs2) & 0x1F));
            break;
            
        case OP_SRL:
            wreg(inst.rd, rreg(inst.rs1) >> (rreg(inst.rs2) & 0x1F));
            break;
            
        case OP_SRA:
            wreg(inst.rd, (i32)rreg(inst.rs1) >> (rreg(inst.rs2) & 0x1F));
            break;
            
        // Переходы
        case OP_JAL:
            wreg(inst.rd, cpu_pc + 4);
            cpu_pc += inst.imm - 4; // -4 потому что после этого будет +4
            break;
            
        case OP_JALR:
            wreg(inst.rd, cpu_pc + 4);
            cpu_pc = (rreg(inst.rs1) + inst.imm) & ~1;
            break;
            
        // Условные переходы
        case OP_BEQ:
            if (rreg(inst.rs1) == rreg(inst.rs2)) cpu_pc += inst.imm - 4;
            break;
            
        case OP_BNE:
            if (rreg(inst.rs1) != rreg(inst.rs2)) cpu_pc += inst.imm - 4;
            break;
            
        case OP_BLT:
            if ((i32)rreg(inst.rs1) < (i32)rreg(inst.rs2)) cpu_pc += inst.imm - 4;
            break;
            
        case OP_BGE:
            if ((i32)rreg(inst.rs1) >= (i32)rreg(inst.rs2)) cpu_pc += inst.imm - 4;
            break;
            
        case OP_BLTU:
            if (rreg(inst.rs1) < rreg(inst.rs2)) cpu_pc += inst.imm - 4;
            break;
            
        case OP_BGEU:
            if (rreg(inst.rs1) >= rreg(inst.rs2)) cpu_pc += inst.imm - 4;
            break;
            
        // Загрузки
        case OP_LB:
            val1 = (i8)mem_read_8(rreg(inst.rs1) + inst.imm);
            wreg(inst.rd, val1);
            break;
            
        case OP_LH:
            val1 = (i16)mem_read_16(rreg(inst.rs1) + inst.imm);
            wreg(inst.rd, val1);
            break;
            
        case OP_LW:
            val1 = mem_read_32(rreg(inst.rs1) + inst.imm);
            wreg(inst.rd, val1);
            break;
            
        case OP_LBU:
            val1 = mem_read_8(rreg(inst.rs1) + inst.imm);
            wreg(inst.rd, val1);
            break;
            
        case OP_LHU:
            val1 = mem_read_16(rreg(inst.rs1) + inst.imm);
            wreg(inst.rd, val1);
            break;
            
        // Сохранения
        case OP_SB:
            mem_write_8(rreg(inst.rs1) + inst.imm, rreg(inst.rs2) & 0xFF);
            break;
            
        case OP_SH:
            mem_write_16(rreg(inst.rs1) + inst.imm, rreg(inst.rs2) & 0xFFFF);
            break;
            
        case OP_SW:
            mem_write_32(rreg(inst.rs1) + inst.imm, rreg(inst.rs2));
            break;
            
        // Системные инструкции
        case OP_ECALL:
            // printf("ECALL instruction executed\n");
            break;
            
        case OP_EBREAK:
            // printf("EBREAK instruction executed\n");
            cpu_running = false;
            break;
            
        case OP_FENCE:
            // Ничего не делаем для простоты
            break;
            
        case OP_FENCE_TSO:
            // Ничего не делаем для простоты
            break;
            
        case OP_PAUSE:
            // Ничего не делаем для простоты
            break;
            
        case OP_UNKNOWN:
            // printf("Unknown instruction at 0x%08X\n", cpu_pc);
            // cpu_err();
            break;
    }
}

void decode_and_execute(CPU_BITS instr) {
    execute(decode(instr));
}
