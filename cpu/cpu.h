#ifndef CPU_CPU_H_
#define CPU_CPU_H_

#include <stdbool.h>
#include <stdint.h>

#define BIT(n) (1u << (n))

#define VECTOR_NMIB 0xFFFA
#define VECTOR_RESET 0xFFFC
#define VECTOR_IRQBRK 0xFFFE

typedef uint16_t addr_t;
typedef uint8_t word_t;
/*
 * Processor status register "P"
 *
 * bit:   7 6 5 4 3 2 1 0
 *        N V 1 B D I Z C
 * reset: * * 1 1 0 1 * *
 *
 * C: Carry 1 = true
 * Z: Zero 1 = true
 * I: IRQB disable 1 = disable
 * D: Decimal mode 1 = true
 * B: BRK command 1 = BRK, 0 = IRQB // ???
 * 1: static
 * V: Overflow 1 = true
 * N: Negative 1 = neg
 */
typedef struct
{
    bool c : 1; /* carry */
    bool z : 1; /* zero */
    bool i : 1; /* IRQB disable */
    bool d : 1; /* decimal mode */
    bool b : 1; /* BRK command */
    bool : 1; /* unused */
    bool v : 1; /* overflow */
    bool n : 1; /* negative */
} procstat_t;

typedef struct
{
    word_t a;
    word_t y;
    word_t x;
    addr_t pc;
    word_t s;
    procstat_t p;
} registers_t;

word_t procstat_to_word(procstat_t p);
procstat_t word_to_procstat(word_t word);

extern registers_t reg;
extern word_t mem[1 << 16];

#endif /* CPU_CPU_H_ */
