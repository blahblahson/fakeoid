#ifndef CPU_MEM_H_
#define CPU_MEM_H_

#include "cpu.h"

/*
 * Memory access functions
 */
uint8_t mem_read(addr_t addr);
void mem_write(addr_t addr, word_t word);

/*
 * Stack manipulation functions
 */
void push(uint8_t word);
void push16(uint16_t dword);
uint8_t pop(void);
uint16_t pop16(void);

/*
 * Utility functions to read a (double) word and advance the program counter.
 * Useful during instruction decoding.
 *
 * TODO: Move this elsewhere. Too specific.
 */
uint8_t shift(void);
uint16_t shift16(void);

#endif /* CPU_MEM_H_ */
