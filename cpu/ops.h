#ifndef CPU_OPS_H_
#define CPU_OPS_H_

#include "cpu.h"

typedef enum
{
    OPERAND_TYPE_NONE,
    OPERAND_TYPE_WORD,
    OPERAND_TYPE_ADDRESS,
    OPERAND_TYPE_ACCUMULATOR,
    OPERAND_TYPE_WORD_ADDRESS, /* In this case, word and address are unrelated */
} operand_type_t;

/*
 * Reference: http://www.obelisk.me.uk/65C02/addressing.html and WDC datasheet
 */
typedef enum
{
    ADDR_MODE_ABSOLUTE, /* Absolute: a */
    ADDR_MODE_ABSOLUTE_INDEXED_INDIRECT, /* Absolute Indexed Indirect: (a,x) */
    ADDR_MODE_ABSOLUTE_X, /* Absolute Indexed with X: a,x */
    ADDR_MODE_ABSOLUTE_Y, /* Absolute Indexed with Y: a,y */
    ADDR_MODE_ABSOLUTE_INDIRECT, /* Absolute Indirect: (a) */
    ADDR_MODE_ACCUMULATOR, /* Accumulator: A */
    ADDR_MODE_IMMEDIATE, /* Immediate: # */
    ADDR_MODE_IMPLIED, /* Implied: i */
    ADDR_MODE_RELATIVE, /* Program Counter Relative: r */
    ADDR_MODE_ZEROPAGE, /* Zero Page: zp */
    ADDR_MODE_ZEROPAGE_INDEXED_INDIRECT, /* Zero Page Indexed Indirect: (zp,x) */
    ADDR_MODE_ZEROPAGE_X, /* Zero Page Indexed with X: zp,x */
    ADDR_MODE_ZEROPAGE_Y, /* Zero Page Indexed with Y: zp,y */
    ADDR_MODE_ZEROPAGE_INDIRECT, /* Zero Page Indirect: (zp) */
    ADDR_MODE_ZEROPAGE_INDIRECT_INDEXED, /* Zero Page Indirect Indexed: (zp),y */
    ADDR_MODE_ZEROPAGE_RELATIVE, /* Zero Page followed by Relative: zp+r */
} addr_mode_t;

typedef struct operand
{
    word_t word;
    addr_t addr;
    operand_type_t type;
} operand_t;

typedef void (*op_handler_t)(operand_t operand);

typedef struct op_desc
{
    op_handler_t handler;
    addr_mode_t addr_mode;
} op_desc_t;

extern op_desc_t ops[];

#endif /* CPU_OPS_H_ */
