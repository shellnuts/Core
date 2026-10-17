#include "bits.h"
#include "unit.h"

static void test_set_clear_toggle(void)
{
    CHECK(bit_set(0x00000000u, 0) == 0x00000001u);
    CHECK(bit_set(0x00000000u, 5) == 0x00000020u);
    CHECK(bit_set(0x00000020u, 5) == 0x00000020u); /* already set */
    CHECK(bit_set(0x00000000u, 31) == 0x80000000u); /* the classic UB trap */

    CHECK(bit_clear(0xFFFFFFFFu, 0) == 0xFFFFFFFEu);
    CHECK(bit_clear(0xFFFFFFFFu, 31) == 0x7FFFFFFFu);
    CHECK(bit_clear(0x00000000u, 7) == 0x00000000u);

    CHECK(bit_toggle(0x0000000Fu, 0) == 0x0000000Eu);
    CHECK(bit_toggle(0x0000000Eu, 0) == 0x0000000Fu);
    CHECK(bit_toggle(0x00000000u, 31) == 0x80000000u);
}

static void test_read(void)
{
    CHECK(bit_read(0x00000001u, 0) == true);
    CHECK(bit_read(0x00000001u, 1) == false);
    CHECK(bit_read(0x80000000u, 31) == true);
    CHECK(bit_read(0x7FFFFFFFu, 31) == false);
    for (unsigned b = 0; b < 32; b++)
        CHECK(bit_read(bit_set(0, b), b));
}

static void test_mask(void)
{
    CHECK(bit_mask(1) == 0x1u);
    CHECK(bit_mask(3) == 0x7u);
    CHECK(bit_mask(16) == 0xFFFFu);
    CHECK(bit_mask(31) == 0x7FFFFFFFu);
    CHECK(bit_mask(32) == 0xFFFFFFFFu); /* 1u << 32 is UB: handle it */
}

static void test_field_get(void)
{
    CHECK(field_get(0x0000ABCDu, 4, 8) == 0xBCu);
    CHECK(field_get(0x0000ABCDu, 0, 4) == 0xDu);
    CHECK(field_get(0xC0000000u, 30, 2) == 0x3u);
    CHECK(field_get(0x12345678u, 0, 32) == 0x12345678u);
}

static void test_field_set(void)
{
    CHECK(field_set(0xFFFFFFFFu, 4, 8, 0x00u) == 0xFFFFF00Fu);
    CHECK(field_set(0x00000000u, 4, 8, 0xA5u) == 0x00000A50u);
    CHECK(field_set(0x00000000u, 4, 4, 0xFFu) == 0x000000F0u); /* extra bits ignored */
    CHECK(field_set(0x00000000u, 30, 2, 0x3u) == 0xC0000000u);
    CHECK(field_set(0xDEADBEEFu, 0, 32, 0x12345678u) == 0x12345678u);

    /* Real use: PF9 to output mode (MODER9 = 01) without touching other pins. */
    uint32_t moder = 0xFFFFFFFFu;
    moder = field_set(moder, 9 * 2, 2, 1);
    CHECK(moder == 0xFFF7FFFFu);
}

int main(void)
{
    RUN(test_set_clear_toggle);
    RUN(test_read);
    RUN(test_mask);
    RUN(test_field_get);
    RUN(test_field_set);
    return UNIT_REPORT();
}
