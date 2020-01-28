#include "cpu.h"

#define STACK(a) ((uint8_t)(0xFF + a))

/*
 * Memory access functions
 *
 * TODO: Here we should yield to the core
 */
uint8_t mem_read(addr_t addr)
{
    return mem[addr];
}

void mem_write(addr_t addr, word_t word)
{
    mem[addr] = word;
}

/*
 * Stack manipulation functions
 */
void push(uint8_t word)
{
    /* TODO: overflow? */
    mem_write(STACK(reg.s--), word);
}

void push16(uint16_t dword)
{
    /* TODO: overflow? */
    mem_write(STACK(reg.s--), dword & 0xFF);
    mem_write(STACK(reg.s--), (dword >> 8) & 0xFF);
}

uint8_t pop(void)
{
    return mem_read(STACK(++reg.s));
}

uint16_t pop16(void)
{
    uint16_t dword = mem_read(STACK(++reg.s)) << 8;
    dword |= mem_read(STACK(++reg.s));
    return dword;
}

/*
 * Utility functions to read a (double) word and advance the program counter.
 * Useful during instruction decoding.
 */
uint8_t shift(void)
{
    return mem_read(reg.pc++);
}

uint16_t shift16(void)
{
    /* 65C02 is little-endian */
    return shift() + (shift() << 8);
}
