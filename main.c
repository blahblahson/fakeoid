/*
 * Read this for info on cycles and timing: http://nparker.llx.com/a2/opcodes.html
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cpu.h"
#include "cpu/mem.h"
#include "cpu/ops.h"

void load_eeprom(void)
{
    /* TODO */
}

void reset(void)
{
    reg.pc = VECTOR_RESET;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    load_eeprom();
    reset();

    while (1)
    {
        uint8_t opcode = mem_read(reg.pc++);
        op_desc_t op = ops[opcode];
        operand_t operand = { .type = OPERAND_TYPE_ADDRESS };

        switch (op.addr_mode)
        {
        case ADDR_MODE_ABSOLUTE:
            operand.addr = shift() + (shift() << 8);
            break;
        case ADDR_MODE_ABSOLUTE_INDEXED_INDIRECT:
            operand.addr = mem_read(shift() + (shift() << 8) + reg.x);
            break;
        case ADDR_MODE_ABSOLUTE_X:
            operand.addr = shift() + (shift() << 8) + reg.x;
            break;
        case ADDR_MODE_ABSOLUTE_Y:
            operand.addr = shift() + (shift() << 8) + reg.y;
            break;
        case ADDR_MODE_ABSOLUTE_INDIRECT:
            operand.addr = mem_read(shift() + (shift() << 8));
            break;
        case ADDR_MODE_ACCUMULATOR:
            operand.type = OPERAND_TYPE_ACCUMULATOR;
            break;
        case ADDR_MODE_IMMEDIATE:
            operand.type = OPERAND_TYPE_WORD;
            operand.word = shift();
            break;
        case ADDR_MODE_IMPLIED:
            operand.type = OPERAND_TYPE_NONE;
            break;
        case ADDR_MODE_RELATIVE:
        {
            /* TODO: figure this out, signedness etc.*/
            int8_t offset = shift();
            operand.addr = reg.pc + offset;
            break;
        }
        case ADDR_MODE_ZEROPAGE:
            operand.addr = shift();
            break;
        case ADDR_MODE_ZEROPAGE_INDEXED_INDIRECT:
            operand.addr = mem_read((uint8_t)(shift() + reg.x));
            break;
        case ADDR_MODE_ZEROPAGE_X:
            operand.addr = (uint8_t)(shift() + reg.x);
            break;
        case ADDR_MODE_ZEROPAGE_Y:
            operand.addr = (uint8_t)(shift() + reg.y);
            break;
        case ADDR_MODE_ZEROPAGE_INDIRECT:
        {
            /* TODO: zp_offset+1 should wrap around? */
            uint8_t zp_offset = shift();
            operand.addr = mem_read(zp_offset) + (mem_read(zp_offset + 1) << 8);
            break;
        }
        case ADDR_MODE_ZEROPAGE_INDIRECT_INDEXED:
        {
            /* TODO: zp_offset+1 should wrap around? */
            uint8_t zp_offset = shift();
            operand.addr = mem_read(zp_offset) + (mem_read(zp_offset + 1) << 8) + reg.y;
            break;
        }
        case ADDR_MODE_ZEROPAGE_RELATIVE:
        {
            operand.type = OPERAND_TYPE_WORD_ADDRESS;
            operand.word = shift();
            /* TODO: figure this out, signedness etc.*/
            int8_t offset = shift();
            operand.addr = reg.pc + offset;
        }
        default:
            break;
        }

        op.handler(operand);
    }

    return 0;
}
