#include "../core/bus.h"
#include "cpu.h"

#define STACK(a) ((uint8_t)(0xFF + a))

/* FIXME: Temporary util functions */
static void cpu_bus_addr_set(uint16_t addr)
{
    for (int i = 0; i < 16; i++) {
            pin_set(&cpu_addr_bus[i], addr & BIT(i));
    }
}

static void cpu_bus_data_set(uint8_t addr)
{
        for (int i = 0; i < 8; i++) {
                pin_set(&cpu_data_bus[i], addr & BIT(i));
        }
}

/*
 * Memory access functions
 *
 * TODO: Here we should yield to the core
 */
uint8_t mem_read(addr_t addr)
{
    uint8_t word = 0;

    for (int i = 0; i < 16; i++)
            pin_set(&cpu_addr_bus[i], addr & BIT(i));

    /* yield */

    /*
     * The idea here is to block on a conditional variable related to the state
     * of the input clock. The core should block on the completion of all
     * modules before advancing the clocks.
     *
     * Another thought - This conditional variable could be defined in terms of
     * pin state.
     */

    for (int i = 0; i < 8; i++)
            word |= pin_evaluate(&cpu_data_bus[i]) >> i;

    return word;
    /* return mem[addr]; */
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
