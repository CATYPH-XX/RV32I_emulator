#ifndef DECODE_H
#define DECODE_H

#include "deft.h"
#include "cpu/rv32i.h"

/*
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

Instruction decode(CPU_BITS instr);
void execute(Instruction inst);
*/

void decode_and_execute(CPU_BITS instr);

#endif