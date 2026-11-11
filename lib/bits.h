/*
 * bits.h - Bit and bit-field helpers.            PHASE 1, EXERCISE 1
 *
 * Every register driver is built from these few operations. Implement them
 * as pure functions (take a value, return the new value) so they can be
 * tested on the PC; on the MCU you'd write e.g.
 *     GPIOF->MODER = field_set(GPIOF->MODER, 9 * 2, 2, 1);
 *
 * Rules: 'bit' is 0..31; 'pos' + 'width' <= 32; 'width' is 1..32.
 * Watch out: in C, (1 << 31) and (1u << 32) are both undefined behaviour.
 * The tests use -fsanitize=undefined and will catch you.
 */
#ifndef BITS_H
#define BITS_H

#include <stdbool.h>
#include <stdint.h>

static inline uint32_t bit_set(uint32_t reg, unsigned bit)
{
    reg |= 1u << bit;
    return reg;
}

static inline uint32_t bit_clear(uint32_t reg, unsigned bit)
{
    reg &= ~(1u << bit);
    return reg;
}

static inline uint32_t bit_toggle(uint32_t reg, unsigned bit)
{

    reg ^= 1u << bit;
    return reg;
}

static inline bool bit_read(uint32_t reg, unsigned bit)
{

    return (reg >> bit) & 1u;
}

/* A mask with the low 'width' bits set: width 3 -> 0b111. Handle width 32! */
static inline uint32_t bit_mask(unsigned width)
{
    return width == 32u ? UINT32_MAX : (1u << width) - 1u;
}

/* Extract the 'width'-bit field starting at bit 'pos', shifted down to bit 0.
 * Example: field_get(0x0000ABCD, 4, 8) == 0xBC */
static inline uint32_t field_get(uint32_t reg, unsigned pos, unsigned width)
{
    return (reg >> pos) & bit_mask(width);
}

/* Replace the field with 'value', leaving every other bit untouched.
 * Bits of 'value' that don't fit in 'width' are ignored (masked off).
 * Example: field_set(0xFFFFFFFF, 4, 8, 0x00) == 0xFFFFF00F */
static inline uint32_t field_set(uint32_t reg, unsigned pos, unsigned width, uint32_t value)
{
    uint32_t mask = bit_mask(width) << pos;
    return (reg & ~mask) | ((value & bit_mask(width)) << pos);
}

#endif
