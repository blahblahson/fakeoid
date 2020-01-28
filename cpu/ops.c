#include "cpu.h"
#include "mem.h"
#include "ops.h"

#include <stdlib.h> /* TODO: remove */

/*
 * Arithmetic utility functions. Pretty much every arithmetic operation sets
 * some bits in the processor status register, and this depends on the value in
 * the accumulator.
 */
static void set_zero()
{
    if (reg.a == 0)
        reg.p.z = 1;
}

static void set_negative()
{
    if (reg.a & BIT(7))
        reg.p.n = 1;
}

/*
 * Memory access abstraction for a given operand_t. Depending on the operand
 * type, a load or store looks different.
 */
static uint8_t load(const operand_t operand)
{
    switch (operand.type)
    {
    case OPERAND_TYPE_WORD:
    case OPERAND_TYPE_WORD_ADDRESS:
        return operand.word;
    case OPERAND_TYPE_ADDRESS:
        return mem_read(operand.addr);
    case OPERAND_TYPE_ACCUMULATOR:
        return reg.a;
    case OPERAND_TYPE_NONE:
    default:
        // TODO: error handling!
        abort();
    }
}

static uint16_t addr(const operand_t operand)
{
    switch (operand.type)
    {
    case OPERAND_TYPE_ADDRESS:
    case OPERAND_TYPE_WORD_ADDRESS:
        return operand.addr;
    case OPERAND_TYPE_NONE:
    case OPERAND_TYPE_WORD:
    case OPERAND_TYPE_ACCUMULATOR:
    default:
        abort();
    }
}

static void store(const operand_t operand, const word_t word)
{
    switch (operand.type)
    {
    case OPERAND_TYPE_ADDRESS:
        mem_write(operand.addr, word);
        break;
    case OPERAND_TYPE_ACCUMULATOR:
        reg.a = word;
        break;
    case OPERAND_TYPE_NONE:
    case OPERAND_TYPE_WORD:
    case OPERAND_TYPE_WORD_ADDRESS:
    default:
        // TODO: error handling
        abort();
    }
}

/*
 * Operand handlers now follow
 */

/* ADC: Add with carry */
static void adc(operand_t operand)
{
    uint8_t carry = reg.p.c;
    uint16_t result;

    result = reg.a + load(operand) + carry;
    reg.a = result & 0xFF;

    /* set carry flag */
    reg.p.c = result & BIT(8);

    /* set overflow flag
     *
     * WDC's programming manual says that the overflow (V) flag is
     * calculated by XORing the carry from bit 7 (which is bit 8) with the
     * carry from bit 6 (which is bit 7). See p. 136. TODO: Check this.
     */
    reg.p.v = (result & BIT(8)) ^ (result & BIT(7));

    set_zero();
    set_negative();
}

/* AND: Logical AND */
static void and (operand_t operand)
{
    reg.a &= load(operand);

    set_zero();
    set_negative();
}

/* ASL: Arithmetic shift left */
static void asl(operand_t operand)
{
    uint16_t result = load(operand) << 1;
    store(operand, result & 0xFF);

    /* set carry flag */
    reg.p.c = result & BIT(8);

    set_zero();
    set_negative();
}

/* BBR: Branch on bit reset */
static void bbr_(operand_t operand, int nr)
{
    /* TODO: review this */
    if (!(load(operand) & BIT(nr)))
        reg.pc = addr(operand);
}

static void bbr0(operand_t operand)
{
    bbr_(operand, 0);
}

static void bbr1(operand_t operand)
{
    bbr_(operand, 1);
}

static void bbr2(operand_t operand)
{
    bbr_(operand, 2);
}

static void bbr3(operand_t operand)
{
    bbr_(operand, 3);
}

static void bbr4(operand_t operand)
{
    bbr_(operand, 4);
}

static void bbr5(operand_t operand)
{
    bbr_(operand, 5);
}

static void bbr6(operand_t operand)
{
    bbr_(operand, 6);
}

static void bbr7(operand_t operand)
{
    bbr_(operand, 7);
}

/* BBS: Branch on bit set */
static void bbs_(operand_t operand, int nr)
{
    /* TODO: review this */
    if (load(operand) & BIT(nr))
        reg.pc = addr(operand);
}

static void bbs0(operand_t operand)
{
    bbs_(operand, 0);
}
static void bbs1(operand_t operand)
{
    bbs_(operand, 1);
}
static void bbs2(operand_t operand)
{
    bbs_(operand, 2);
}
static void bbs3(operand_t operand)
{
    bbs_(operand, 3);
}
static void bbs4(operand_t operand)
{
    bbs_(operand, 4);
}
static void bbs5(operand_t operand)
{
    bbs_(operand, 5);
}
static void bbs6(operand_t operand)
{
    bbs_(operand, 6);
}
static void bbs7(operand_t operand)
{
    bbs_(operand, 7);
}

/* BCC: Branch if carry clear */
static void bcc(operand_t operand)
{
    if (!reg.p.c)
        reg.pc = addr(operand);
}

/* BCS: Branch if carry set */
static void bcs(operand_t operand)
{
    if (reg.p.c)
        reg.pc = addr(operand);
}

/* BEQ: Branch if equal */
static void beq(operand_t operand)
{
    if (reg.p.z)
        reg.pc = addr(operand);
}

/* BIT: Bit test */
static void bit(operand_t operand)
{
    uint8_t value = load(operand);
    uint8_t result = reg.a & value;

    reg.p.z = !result;

    reg.p.v = value & BIT(6);
    reg.p.n = value & BIT(7);
}

/* BMI: Branch if minus */
static void bmi(operand_t operand)
{
    if (reg.p.n)
        reg.pc = addr(operand);
}

/* BNI: Branch if not equal */
static void bni(operand_t operand)
{
    if (!reg.p.z)
        reg.pc = addr(operand);
}

/* BPL: Branch if positive */
static void bpl(operand_t operand)
{
    if (!reg.p.n)
        reg.pc = addr(operand);
}

/* BRA: Branch always */
static void bra(operand_t operand)
{
    (void)operand;
    reg.pc = addr(operand);
}

/* BRK: Force interrupt */
static void brk(operand_t operand)
{
    (void)operand;
    push16(reg.pc);
    push(procstat_to_word(reg.p));
    reg.p.i = 1;
    reg.pc = VECTOR_IRQBRK;
}

/* BVC: Branch if overflow clear */
static void bvc(operand_t operand)
{
    if (!reg.p.v)
        reg.pc = addr(operand);
}

/* BVS: Branch if overflow set */
static void bvs(operand_t operand)
{
    if (reg.p.v)
        reg.pc = addr(operand);
}

/* CLC: Clear carry flag */
static void clc(operand_t operand)
{
    (void)operand;
    reg.p.c = 0;
}

/* CLD: Clear decimal mode */
static void cld(operand_t operand)
{
    (void)operand;
    reg.p.d = 0;
}

/* CLI: Clear interrupt disable */
static void cli(operand_t operand)
{
    (void)operand;
    reg.p.i = 0;
}

/* CLV: Clear overflow flag */
static void clv(operand_t operand)
{
    (void)operand;
    reg.p.v = 0;
}

/* CMP: Compare */
static void cmp(operand_t operand)
{
    int16_t result = reg.a - load(operand);

    reg.p.c = result >= 0;
    reg.p.z = result == 0;
    reg.p.n = result & BIT(7);
}

/* CPX: Compare X register */
static void cpx(operand_t operand)
{
    int16_t result = reg.x - load(operand);

    reg.p.c = result >= 0;
    reg.p.z = result == 0;
    reg.p.n = result & BIT(7);
}

/* CPY: Compare Y register */
static void cpy(operand_t operand)
{
    int16_t result = reg.y - load(operand);

    reg.p.c = result >= 0;
    reg.p.z = result == 0;
    reg.p.n = result & BIT(7);
}

/* DEC: Decrement memory */
static void dec(operand_t operand)
{
    uint8_t result = load(operand) - 1;

    reg.p.z = !result;
    reg.p.n = result & BIT(7);
    store(operand, result);
}

/* DEX: Decrement X register */
static void dex(operand_t operand)
{
    (void)operand;
    reg.x--;

    reg.p.z = !reg.x;
    reg.p.n = reg.x & BIT(7);
}

/* DEY: Decrement Y register */
static void dey(operand_t operand)
{
    (void)operand;
    reg.y--;

    reg.p.z = !reg.y;
    reg.p.n = reg.y & BIT(7);
}

/* EOR: Exclusive OR */
static void eor(operand_t operand)
{
    reg.a ^= load(operand);

    reg.p.z = !reg.a;
    reg.p.n = reg.a & BIT(7);
}

/* INC: Increment memory */
static void inc(operand_t operand)
{
    uint8_t result = load(operand) + 1;

    reg.p.z = !result;
    reg.p.n = result & BIT(7);
    store(operand, result);
}

/* INX: Increment X register */
static void inx(operand_t operand)
{
    (void)operand;
    reg.x++;

    reg.p.z = !reg.x;
    reg.p.n = reg.x & BIT(7);
}

/* INY: Increment Y register */
static void iny(operand_t operand)
{
    (void)operand;
    reg.y++;

    reg.p.z = !reg.y;
    reg.p.n = reg.y & BIT(7);
}

/* JMP: Jump */
static void jmp(operand_t operand)
{
    reg.pc = addr(operand);
}

/* JSR: Jump to subroutine */
static void jsr(operand_t operand)
{
    push16(reg.pc - 1);
    reg.pc = addr(operand);
}

/* LDA: Load accumulator */
/* NOTE:
 * https://retrocomputing.stackexchange.com/questions/145/why-does-6502-indexed-lda-take-an-extra-cycle-at-page-boundaries
 */
static void lda(operand_t operand)
{
    reg.a = load(operand);

    reg.p.z = !reg.a;
    reg.p.n = reg.a & BIT(7);
}

/* LDX: Load X register */
static void ldx(operand_t operand)
{
    reg.x = load(operand);

    reg.p.z = !reg.x;
    reg.p.n = reg.x & BIT(7);
}

/* LDY: Load Y register */
static void ldy(operand_t operand)
{
    reg.y = load(operand);

    reg.p.z = !reg.y;
    reg.p.n = reg.y & BIT(7);
}

/* LSR: Logical shift right */
static void lsr(operand_t operand)
{
    uint8_t word = load(operand);

    reg.p.c = word & BIT(0);
    reg.a = word >> 1;
    reg.p.z = !reg.a;
    reg.p.n = reg.a & BIT(7);
}

/* NOP: No operation */
static void nop(operand_t operand)
{
    (void)operand;
}

/* ORA: Logical inclusive OR */
static void ora(operand_t operand)
{
    reg.a |= load(operand);

    reg.p.z = !reg.a;
    reg.p.n = reg.a & BIT(7);
}

/* PHA: Push accumulator */
static void pha(operand_t operand)
{
    (void)operand;
    push(reg.a);
}

/* PHP: Push processor status */
static void php(operand_t operand)
{
    (void)operand;
    push(procstat_to_word(reg.p));
}

/* PHX: Push X register */
static void phx(operand_t operand)
{
    (void)operand;
    push(reg.x);
}

/* PHY: Push Y register */
static void phy(operand_t operand)
{
    (void)operand;
    push(reg.y);
}

/* PLA: Pull accumulator */
static void pla(operand_t operand)
{
    (void)operand;
    reg.a = pop();

    reg.p.z = !reg.a;
    reg.p.n = reg.a & BIT(7);
}

/* PLP: Pull processor status */
static void plp(operand_t operand)
{
    (void)operand;
    reg.p = word_to_procstat(pop());
}

/* PLX: Pull X register */
static void plx(operand_t operand)
{
    (void)operand;
    reg.x = pop();

    reg.p.z = !reg.x;
    reg.p.n = reg.x & BIT(7);
}

/* PLY: Pull Y register */
static void ply(operand_t operand)
{
    (void)operand;
    reg.y = pop();

    reg.p.z = !reg.y;
    reg.p.n = reg.y & BIT(7);
}

/* RMB: Reset memory bit */
static void rmb_(operand_t operand, int nr)
{
    store(operand, load(operand) & ~BIT(nr));
}

static void rmb0(operand_t operand)
{
    rmb_(operand, 0);
}

static void rmb1(operand_t operand)
{
    rmb_(operand, 1);
}

static void rmb2(operand_t operand)
{
    rmb_(operand, 2);
}

static void rmb3(operand_t operand)
{
    rmb_(operand, 3);
}

static void rmb4(operand_t operand)
{
    rmb_(operand, 4);
}

static void rmb5(operand_t operand)
{
    rmb_(operand, 5);
}

static void rmb6(operand_t operand)
{
    rmb_(operand, 6);
}

static void rmb7(operand_t operand)
{
    rmb_(operand, 7);
}

/* ROL: Rotate left */
static void rol(operand_t operand)
{
    uint16_t result = (load(operand) << 1) | reg.p.c;

    reg.p.c = result & BIT(8);
    reg.p.z = !(result & 0xFF); /* restrict test to lower 8 bits */
    reg.p.n = result & BIT(7);
    store(operand, result & 0xFF);
}

/* ROR: Rotate right */
static void ror(operand_t operand)
{
    uint8_t tmp = load(operand);
    uint8_t result = (tmp >> 1) | (reg.p.c << 7);

    reg.p.c = tmp & BIT(0);
    reg.p.z = !result;
    reg.p.n = result & BIT(7);
    store(operand, result);
}

/* RTI: Return from interrupt */
static void rti(operand_t operand)
{
    (void)operand;
    reg.p = word_to_procstat(pop());
    reg.pc = pop16();
}

/* RTS: Return from subroutine */
static void rts(operand_t operand)
{
    (void)operand;
    reg.pc = pop16() + 1;
}

/* SBC: Subtract with carry */
static void sbc(operand_t operand)
{
    uint8_t carry = reg.p.c;
    uint16_t result;

    result = reg.a - load(operand) - (1 - carry);
    reg.a = result & 0xFF;

    /* set carry flag */
    reg.p.c = result & BIT(8);

    /* set overflow flag
     *
     * This follows the same XORing logic as in ADC.
     */
    reg.p.v = (result & BIT(8)) ^ (result & BIT(7));

    reg.p.z = !result;
    reg.p.n = result & BIT(7);
}

/* SEC: Set carry flag */
static void sec(operand_t operand)
{
    (void)operand;
    reg.p.c = 1;
}

/* SED: Set decimal flag */
static void sed(operand_t operand)
{
    (void)operand;
    reg.p.d = 1;
}

/* SEI: Set interrupt disable */
static void sei(operand_t operand)
{
    (void)operand;
    reg.p.i = 1;
}

/* SMB: Set memory bit */
static void smb_(operand_t operand, int nr)
{
    store(operand, load(operand) | BIT(nr));
}

static void smb0(operand_t operand)
{
    smb_(operand, 0);
}

static void smb1(operand_t operand)
{
    smb_(operand, 1);
}

static void smb2(operand_t operand)
{
    smb_(operand, 2);
}

static void smb3(operand_t operand)
{
    smb_(operand, 3);
}

static void smb4(operand_t operand)
{
    smb_(operand, 4);
}

static void smb5(operand_t operand)
{
    smb_(operand, 5);
}

static void smb6(operand_t operand)
{
    smb_(operand, 6);
}

static void smb7(operand_t operand)
{
    smb_(operand, 7);
}

/* STA: Store accumulator */
static void sta(operand_t operand)
{
    store(operand, reg.a);
}

/* STP: Stop */
static void stp(operand_t operand)
{
    (void)operand;
    /* TODO: Implement this */
}

/* STX: Store X register */
static void stx(operand_t operand)
{
    store(operand, reg.x);
}

/* STY: Store Y register */
static void sty(operand_t operand)
{
    store(operand, reg.y);
}

/* STZ: Store zero */
static void stz(operand_t operand)
{
    store(operand, 0);
}

/* TAX: Transfer accumulator to X */
static void tax(operand_t operand)
{
    (void)operand;
    reg.x = reg.a;

    reg.p.z = !reg.x;
    reg.p.n = reg.x & BIT(7);
}

/* TAY: Transfer accumulator to Y */
static void tay(operand_t operand)
{
    (void)operand;
    reg.y = reg.a;

    reg.p.z = !reg.y;
    reg.p.n = reg.y & BIT(7);
}

/* TRB: Test and reset bits */
static void trb(operand_t operand)
{
    uint8_t word = load(operand);

    reg.p.z = word & reg.a;
    store(operand, word & ~reg.a);
}

/* TSB: Test and set bits */
static void tsb(operand_t operand)
{
    uint8_t word = load(operand);

    reg.p.z = word & reg.a;
    store(operand, word | reg.a);
}

/* TSX: Transfer stack pointer to X */
static void tsx(operand_t operand)
{
    (void)operand;
    reg.x = reg.s;

    reg.p.z = !reg.x;
    reg.p.n = reg.x & BIT(7);
}

/* TSY: Transfer stack pointer to Y */
static void tsy(operand_t operand)
{
    (void)operand;
    reg.y = reg.s;

    reg.p.z = !reg.y;
    reg.p.n = reg.y & BIT(7);
}

/* TXS: Transfer X to stack pointer */
static void txs(operand_t operand)
{
    (void)operand;
    reg.s = reg.x;
}

/* TYA: Transfer Y to accumulator */
static void tya(operand_t operand)
{
    (void)operand;
    reg.a = reg.y;

    reg.p.z = !reg.y;
    reg.p.n = reg.a & BIT(7);
}

/* WAI: Wait for interrupt */
static void wai(operand_t operand)
{
    (void)operand;
    /* TODO: Implement this */
    reg.p.b = 1;
}

op_desc_t ops[] = {
    /*
     * TODO: Fill in the "blanks" with NOP
     */
    [0x69] = { adc, ADDR_MODE_IMMEDIATE },
    [0x65] = { adc, ADDR_MODE_ZEROPAGE },
    [0x75] = { adc, ADDR_MODE_ZEROPAGE_X },
    [0x6D] = { adc, ADDR_MODE_ABSOLUTE },
    [0x7D] = { adc, ADDR_MODE_ABSOLUTE_X },
    [0x79] = { adc, ADDR_MODE_ABSOLUTE_Y },
    [0x61] = { adc, ADDR_MODE_ZEROPAGE_INDEXED_INDIRECT },
    [0x71] = { adc, ADDR_MODE_ZEROPAGE_INDIRECT_INDEXED },
    [0x72] = { adc, ADDR_MODE_ZEROPAGE_INDIRECT },
    [0x29] = { and, ADDR_MODE_IMMEDIATE },
    [0x25] = { and, ADDR_MODE_ZEROPAGE },
    [0x35] = { and, ADDR_MODE_ZEROPAGE_X },
    [0x2D] = { and, ADDR_MODE_ABSOLUTE },
    [0x3D] = { and, ADDR_MODE_ABSOLUTE_X },
    [0x39] = { and, ADDR_MODE_ABSOLUTE_Y },
    [0x21] = { and, ADDR_MODE_ZEROPAGE_INDEXED_INDIRECT },
    [0x31] = { and, ADDR_MODE_ZEROPAGE_INDIRECT_INDEXED },
    [0x32] = { and, ADDR_MODE_ZEROPAGE_INDIRECT },
    [0x0A] = { asl, ADDR_MODE_ACCUMULATOR },
    [0x06] = { asl, ADDR_MODE_ZEROPAGE },
    [0x16] = { asl, ADDR_MODE_ZEROPAGE_X },
    [0x0E] = { asl, ADDR_MODE_ABSOLUTE },
    [0x1E] = { asl, ADDR_MODE_ABSOLUTE_X },
    /*
     * TODO: Add now BBRn instructions, which actually use a combination of the
     * addressing modes zero page and relative: zp,r. TODO: Check if the 3rd
     * byte (relative) is even read if branch condition is false. See
     * http://6502.org/tutorials/65c02opcodes.html for more info.
     */
    [0x0F] = { bbr0, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0x1F] = { bbr1, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0x2F] = { bbr2, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0x3F] = { bbr3, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0x4F] = { bbr4, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0x5F] = { bbr5, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0x6F] = { bbr6, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0x7F] = { bbr7, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0x8F] = { bbs0, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0x9F] = { bbs1, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0xAF] = { bbs2, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0xBF] = { bbs3, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0xCF] = { bbs4, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0xDF] = { bbs5, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0xEF] = { bbs6, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0xFF] = { bbs7, ADDR_MODE_ZEROPAGE_RELATIVE },
    [0x90] = { bcc, ADDR_MODE_RELATIVE },
    [0xB0] = { bcs, ADDR_MODE_RELATIVE },
    [0xF0] = { beq, ADDR_MODE_RELATIVE },
    [0x30] = { bmi, ADDR_MODE_RELATIVE },
    [0xD0] = { bni, ADDR_MODE_RELATIVE },
    [0x10] = { bpl, ADDR_MODE_RELATIVE },
    [0x80] = { bra, ADDR_MODE_RELATIVE },
    [0x50] = { bvc, ADDR_MODE_RELATIVE },
    [0x70] = { bvs, ADDR_MODE_RELATIVE },
    [0x89] = { bit, ADDR_MODE_IMMEDIATE },
    [0x24] = { bit, ADDR_MODE_ZEROPAGE },
    [0x34] = { bit, ADDR_MODE_ZEROPAGE },
    [0x2C] = { bit, ADDR_MODE_ABSOLUTE },
    [0x3C] = { bit, ADDR_MODE_ABSOLUTE_X },
    [0x00] = { brk, ADDR_MODE_IMPLIED },
    [0x18] = { clc, ADDR_MODE_IMPLIED },
    [0xD8] = { cld, ADDR_MODE_IMPLIED },
    [0x58] = { cli, ADDR_MODE_IMPLIED },
    [0xB8] = { clv, ADDR_MODE_IMPLIED },
    [0xC9] = { cmp, ADDR_MODE_IMMEDIATE },
    [0xC5] = { cmp, ADDR_MODE_ZEROPAGE },
    [0xD5] = { cmp, ADDR_MODE_ZEROPAGE_X },
    [0xCD] = { cmp, ADDR_MODE_ABSOLUTE },
    [0xDD] = { cmp, ADDR_MODE_ABSOLUTE_X },
    [0xD9] = { cmp, ADDR_MODE_ABSOLUTE_Y },
    [0xC1] = { cmp, ADDR_MODE_ZEROPAGE_INDEXED_INDIRECT },
    [0xD1] = { cmp, ADDR_MODE_ZEROPAGE_INDIRECT_INDEXED },
    [0xD2] = { cmp, ADDR_MODE_ZEROPAGE_INDIRECT },
    [0xE0] = { cpx, ADDR_MODE_IMMEDIATE },
    [0xE4] = { cpx, ADDR_MODE_ZEROPAGE },
    [0xEC] = { cpx, ADDR_MODE_ABSOLUTE },
    [0xC0] = { cpy, ADDR_MODE_IMMEDIATE },
    [0xC4] = { cpy, ADDR_MODE_ZEROPAGE },
    [0xCC] = { cpy, ADDR_MODE_ABSOLUTE },
    [0x3A] = { dec, ADDR_MODE_ACCUMULATOR },
    [0xC6] = { dec, ADDR_MODE_ZEROPAGE },
    [0xD6] = { dec, ADDR_MODE_ZEROPAGE_X },
    [0xCE] = { dec, ADDR_MODE_ABSOLUTE },
    [0xDE] = { dec, ADDR_MODE_ABSOLUTE_X },
    [0xCA] = { dex, ADDR_MODE_IMPLIED },
    [0x88] = { dey, ADDR_MODE_IMPLIED },
    [0x49] = { eor, ADDR_MODE_IMMEDIATE },
    [0x45] = { eor, ADDR_MODE_ZEROPAGE },
    [0x55] = { eor, ADDR_MODE_ZEROPAGE_X },
    [0x4D] = { eor, ADDR_MODE_ABSOLUTE },
    [0x5D] = { eor, ADDR_MODE_ABSOLUTE_X },
    [0x59] = { eor, ADDR_MODE_ABSOLUTE_Y },
    [0x41] = { eor, ADDR_MODE_ZEROPAGE_INDEXED_INDIRECT },
    [0x51] = { eor, ADDR_MODE_ZEROPAGE_INDIRECT_INDEXED },
    [0x52] = { eor, ADDR_MODE_ZEROPAGE_INDIRECT },
    [0x1A] = { inc, ADDR_MODE_ACCUMULATOR },
    [0xE6] = { inc, ADDR_MODE_ZEROPAGE },
    [0xF6] = { inc, ADDR_MODE_ZEROPAGE_X },
    [0xEE] = { inc, ADDR_MODE_ABSOLUTE },
    [0xFE] = { inc, ADDR_MODE_ABSOLUTE_X },
    [0xE8] = { inx, ADDR_MODE_IMPLIED },
    [0xC8] = { iny, ADDR_MODE_IMPLIED },
    [0x4C] = { jmp, ADDR_MODE_ABSOLUTE },
    [0x6C] = { jmp, ADDR_MODE_ABSOLUTE_INDIRECT },
    [0x7C] = { jmp, ADDR_MODE_ABSOLUTE_INDEXED_INDIRECT },
    [0x20] = { jsr, ADDR_MODE_ABSOLUTE },
    [0xA9] = { lda, ADDR_MODE_IMMEDIATE },
    [0xA5] = { lda, ADDR_MODE_ZEROPAGE },
    [0xB5] = { lda, ADDR_MODE_ZEROPAGE_X },
    [0xAD] = { lda, ADDR_MODE_ABSOLUTE },
    [0xBD] = { lda, ADDR_MODE_ABSOLUTE_X },
    [0xB9] = { lda, ADDR_MODE_ABSOLUTE_Y },
    [0xA1] = { lda, ADDR_MODE_ZEROPAGE_INDEXED_INDIRECT },
    [0xB1] = { lda, ADDR_MODE_ZEROPAGE_INDIRECT_INDEXED },
    [0xB2] = { lda, ADDR_MODE_ZEROPAGE_INDIRECT },
    [0xA2] = { ldx, ADDR_MODE_IMMEDIATE },
    [0xA6] = { ldx, ADDR_MODE_ZEROPAGE },
    [0xB6] = { ldx, ADDR_MODE_ZEROPAGE_Y },
    [0xAE] = { ldx, ADDR_MODE_ABSOLUTE },
    [0xBE] = { ldx, ADDR_MODE_ABSOLUTE_Y },
    [0xA0] = { ldy, ADDR_MODE_IMMEDIATE },
    [0xA4] = { ldy, ADDR_MODE_ZEROPAGE },
    [0xB4] = { ldy, ADDR_MODE_ZEROPAGE_X },
    [0xAC] = { ldy, ADDR_MODE_ABSOLUTE },
    [0xBC] = { ldy, ADDR_MODE_ABSOLUTE_X },
    [0x4A] = { lsr, ADDR_MODE_ACCUMULATOR, },
    [0x46] = { lsr, ADDR_MODE_ZEROPAGE },
    [0x56] = { lsr, ADDR_MODE_ZEROPAGE_X },
    [0x5E] = { lsr, ADDR_MODE_ABSOLUTE_X },
    [0xEA] = { nop, ADDR_MODE_IMPLIED },
    [0x09] = { ora, ADDR_MODE_IMMEDIATE },
    [0x05] = { ora, ADDR_MODE_ZEROPAGE },
    [0x15] = { ora, ADDR_MODE_ZEROPAGE_X },
    [0x0D] = { ora, ADDR_MODE_ABSOLUTE },
    [0x1D] = { ora, ADDR_MODE_ABSOLUTE_X },
    [0x19] = { ora, ADDR_MODE_ABSOLUTE_Y },
    [0x01] = { ora, ADDR_MODE_ZEROPAGE_INDEXED_INDIRECT },
    [0x11] = { ora, ADDR_MODE_ZEROPAGE_INDIRECT_INDEXED },
    [0x12] = { ora, ADDR_MODE_ZEROPAGE_INDIRECT },
    [0x48] = { pha, ADDR_MODE_IMPLIED },
    [0x08] = { php, ADDR_MODE_IMPLIED },
    [0xDA] = { phx, ADDR_MODE_IMPLIED },
    [0x5A] = { phy, ADDR_MODE_IMPLIED },
    [0x68] = { pla, ADDR_MODE_IMPLIED },
    [0x28] = { plp, ADDR_MODE_IMPLIED },
    [0xFA] = { plx, ADDR_MODE_IMPLIED },
    [0x7A] = { ply, ADDR_MODE_IMPLIED },
    [0x07] = { rmb0, ADDR_MODE_ZEROPAGE },
    [0x17] = { rmb1, ADDR_MODE_ZEROPAGE },
    [0x27] = { rmb2, ADDR_MODE_ZEROPAGE },
    [0x37] = { rmb3, ADDR_MODE_ZEROPAGE },
    [0x47] = { rmb4, ADDR_MODE_ZEROPAGE },
    [0x57] = { rmb5, ADDR_MODE_ZEROPAGE },
    [0x67] = { rmb6, ADDR_MODE_ZEROPAGE },
    [0x77] = { rmb7, ADDR_MODE_ZEROPAGE },
    [0x2A] = { rol, ADDR_MODE_ACCUMULATOR },
    [0x26] = { rol, ADDR_MODE_ZEROPAGE },
    [0x36] = { rol, ADDR_MODE_ZEROPAGE_X },
    [0x2E] = { rol, ADDR_MODE_ABSOLUTE },
    [0x3E] = { rol, ADDR_MODE_ABSOLUTE_X },
    [0x6A] = { ror, ADDR_MODE_ACCUMULATOR },
    [0x66] = { ror, ADDR_MODE_ZEROPAGE },
    [0x76] = { ror, ADDR_MODE_ZEROPAGE_X },
    [0x6E] = { ror, ADDR_MODE_ABSOLUTE },
    [0x7E] = { ror, ADDR_MODE_ABSOLUTE_X },
    [0x40] = { rti, ADDR_MODE_IMPLIED },
    [0x60] = { rts, ADDR_MODE_IMPLIED },
    [0xE9] = { sbc, ADDR_MODE_IMMEDIATE },
    [0xE5] = { sbc, ADDR_MODE_ZEROPAGE },
    [0xF5] = { sbc, ADDR_MODE_ZEROPAGE_X },
    [0xED] = { sbc, ADDR_MODE_ABSOLUTE },
    [0xFD] = { sbc, ADDR_MODE_ABSOLUTE_X },
    [0xF9] = { sbc, ADDR_MODE_ABSOLUTE_Y },
    [0xE1] = { sbc, ADDR_MODE_ZEROPAGE_INDEXED_INDIRECT },
    [0xF1] = { sbc, ADDR_MODE_ZEROPAGE_INDIRECT_INDEXED },
    [0xF2] = { sbc, ADDR_MODE_ZEROPAGE_INDIRECT },
    [0x38] = { sec, ADDR_MODE_IMPLIED },
    [0xF8] = { sed, ADDR_MODE_IMPLIED },
    [0x78] = { sei, ADDR_MODE_IMPLIED },
    [0x87] = { smb0, ADDR_MODE_ZEROPAGE },
    [0x97] = { smb1, ADDR_MODE_ZEROPAGE },
    [0xA7] = { smb2, ADDR_MODE_ZEROPAGE },
    [0xB7] = { smb3, ADDR_MODE_ZEROPAGE },
    [0xC7] = { smb4, ADDR_MODE_ZEROPAGE },
    [0xD7] = { smb5, ADDR_MODE_ZEROPAGE },
    [0xE7] = { smb6, ADDR_MODE_ZEROPAGE },
    [0xF7] = { smb7, ADDR_MODE_ZEROPAGE },
    [0x85] = { sta, ADDR_MODE_ZEROPAGE },
    [0x95] = { sta, ADDR_MODE_ZEROPAGE_X },
    [0x8D] = { sta, ADDR_MODE_ABSOLUTE },
    [0x9D] = { sta, ADDR_MODE_ABSOLUTE_X },
    [0x99] = { sta, ADDR_MODE_ABSOLUTE_Y },
    [0x81] = { sta, ADDR_MODE_ZEROPAGE_INDEXED_INDIRECT },
    [0x91] = { sta, ADDR_MODE_ZEROPAGE_INDIRECT_INDEXED },
    [0x92] = { sta, ADDR_MODE_ZEROPAGE_INDIRECT },
    [0xDB] = { stp, ADDR_MODE_IMPLIED },
    [0x86] = { stx, ADDR_MODE_ZEROPAGE },
    [0x96] = { stx, ADDR_MODE_ZEROPAGE_Y },
    [0x8E] = { stx, ADDR_MODE_ABSOLUTE },
    [0x84] = { sty, ADDR_MODE_ZEROPAGE },
    [0x94] = { sty, ADDR_MODE_ZEROPAGE_X },
    [0x8C] = { sty, ADDR_MODE_ABSOLUTE },
    [0x64] = { stz, ADDR_MODE_ZEROPAGE },
    [0x74] = { stz, ADDR_MODE_ZEROPAGE_X },
    [0x9C] = { stz, ADDR_MODE_ABSOLUTE },
    [0x9E] = { stz, ADDR_MODE_ABSOLUTE_X },
    [0xAA] = { tax, ADDR_MODE_IMPLIED },
    [0xA8] = { tay, ADDR_MODE_IMPLIED },
    [0x14] = { trb, ADDR_MODE_ZEROPAGE },
    [0x1C] = { trb, ADDR_MODE_ABSOLUTE },
    [0x04] = { tsb, ADDR_MODE_ZEROPAGE },
    [0x0C] = { tsb, ADDR_MODE_ABSOLUTE },
    [0xBA] = { tsx, ADDR_MODE_IMPLIED },
    [0x8A] = { tsy, ADDR_MODE_IMPLIED },
    [0x9A] = { txs, ADDR_MODE_IMPLIED },
    [0x98] = { tya, ADDR_MODE_IMPLIED },
    [0xCB] = { wai, ADDR_MODE_IMPLIED },
};
