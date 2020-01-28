#include "cpu.h"

registers_t reg = { 0 };
uint8_t mem[1 << 16] = { 0 };

/*
 * Order of bitfields is implementation defined, so we need a helper function
 */
uint8_t procstat_to_word(procstat_t p)
{
    uint8_t word = 0;
    word |= p.c << 0;
    word |= p.z << 1;
    word |= p.i << 2;
    word |= p.d << 3;
    word |= p.b << 4;
    /* bit 5 is unused */
    word |= p.v << 6;
    word |= p.n << 7;

    return word;
}

procstat_t word_to_procstat(uint8_t word)
{
    return (procstat_t){
        .c = word & BIT(0),
        .z = word & BIT(1),
        .i = word & BIT(2),
        .d = word & BIT(3),
        .b = word & BIT(4),
        /* bit 5 is unused */
        .v = word & BIT(6),
        .n = word & BIT(7),
    };
}
