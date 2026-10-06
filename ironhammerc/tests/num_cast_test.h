#ifndef IRON_HAMMER_C_TESTS_NUM_CAST_TEST_H
#define IRON_HAMMER_C_TESTS_NUM_CAST_TEST_H

#include "ic_hammer.h"
#include "docs/premade/numbers.h"
#include <math.h>

IHC_TEST(verify_simple_same_bit_cast_gives_expected_value)
{
    const i32 integer = 42;
    const f32 floating = 42.1337f;
    const i32 int_from_float = cast_f32_to_i32(floating);
    IHC_CHECK(integer == int_from_float);
}

IHC_TEST(verify_floating_to_unsigned_stays_within_bounds)
{
    const f64 too_big = 5555555.5555;
    const u8 u8_max = UINT8_MAX;
    const u8 big_to_u8 = cast_f64_to_u8(too_big);
    IHC_CHECK(u8_max == big_to_u8);

    const f64 negative = (-1.0) * too_big;
    const u8 neg_to_u8 = cast_f64_to_u8(negative);
    IHC_CHECK(0u == neg_to_u8);
}

IHC_TEST(verify_floating_infinity_and_nan_becomes_bounded)
{
    // 32-bit floating point
    const f32 f32_max = FLT_MAX;
    const f32 f32_min = -FLT_MAX;
    const f32 pos_inf_to_f32 = cast_f32_to_f32(INFINITY);
    IHC_CHECK(f32_max == pos_inf_to_f32);
    const f32 neg_inf_to_f32 = cast_f32_to_f32(-INFINITY);
    IHC_CHECK(f32_min == neg_inf_to_f32);
    const f32 NaN_to_f32 = cast_f32_to_f32(NAN);
    IHC_CHECK(f32_min == NaN_to_f32);

    // 64-bit floating point
    const f64 f64_max = DBL_MAX;
    const f64 f64_min = -DBL_MAX;
    const f64 pos_inf_to_f64 = cast_f64_to_f64(INFINITY);
    IHC_CHECK(f64_max == pos_inf_to_f64);
    const f64 neg_inf_to_f64 = cast_f64_to_f64(-INFINITY);
    IHC_CHECK(f64_min == neg_inf_to_f64);
    const f64 NaN_to_f64 = cast_f64_to_f64(NAN);
    IHC_CHECK(f64_min == NaN_to_f64);
}

IHC_TEST(verify_signed_integer_conversions_for_large_type_to_small_type)
{
    const i64 i64_max = INT64_MAX;
    const i64 i64_min = INT64_MIN;

    // i64 to i16
    const i16 i16_max = INT16_MAX;
    const i16 i16_min = INT16_MIN;
    const i16 arb_to_i16 = cast_i64_to_i16(789);
    IHC_CHECK(arb_to_i16 == 789);
    const i16 big_to_i16 = cast_i64_to_i16(i64_max);
    IHC_CHECK(i16_max == big_to_i16);
    const i16 small_to_i16 = cast_i64_to_i16(i64_min);
    IHC_CHECK(i16_min == small_to_i16);

    // i64 to u16
    const u16 u16_max = UINT16_MAX;
    const u16 arb_to_u16 = cast_i64_to_u16(789);
    IHC_CHECK(arb_to_u16 == 789);
    const u16 big_to_u16 = cast_i64_to_u16(i64_max);
    IHC_CHECK(u16_max == big_to_u16);
    const u16 small_to_u16 = cast_i64_to_u16(i64_min);
    IHC_CHECK(0u == small_to_u16);

    // i64 to f32
    const f32 arb_to_f32 = cast_i64_to_f32(789);
    IHC_CHECK(arb_to_f32 == 789.0f);
    const f32 big_to_f32 = cast_i64_to_f32(i64_max);
    const f32 small_to_f32 = cast_i64_to_f32(i64_min);
    // note f32 max is bigger than i64 max
    IHC_CHECK(big_to_f32 == i64_max); 
    IHC_CHECK(small_to_f32 == i64_min);
}

IHC_TEST(verify_unsigned_integer_conversions_for_large_type_to_small_type)
{
    const u64 u64_max = UINT64_MAX;

    // u64 to i16
    const i16 i16_max = INT16_MAX;
    const i16 arb_to_i16 = cast_u64_to_i16(789);
    IHC_CHECK(arb_to_i16 == 789);
    const i16 big_to_i16 = cast_u64_to_i16(u64_max);
    IHC_CHECK(i16_max == big_to_i16);

    // u64 to u16
    const u16 u16_max = UINT16_MAX;
    const u16 arb_to_u16 = cast_u64_to_u16(789);
    IHC_CHECK(arb_to_u16 == 789);
    const u16 big_to_u16 = cast_u64_to_u16(u64_max);
    IHC_CHECK(u16_max == big_to_u16);

    // u64 to f32
    const f32 arb_to_f32 = cast_u64_to_f32(789);
    IHC_CHECK(arb_to_f32 == 789.0f);
    const f32 big_to_f32 = cast_u64_to_f32(u64_max);
    // cast functions forbid undefined behavior, underflow, and overflow, but allow precision loss
    IHC_CHECK((0.999f * u64_max) <= big_to_f32 && big_to_f32 <= (1.001f * u64_max));
}

IHC_TEST(verify_floating_point_conversions_for_large_type_to_small_type)
{
    const f64 f64_max = DBL_MAX;
    const f64 f64_min = -DBL_MAX;

    // f64 to i16
    const i16 i16_max = INT16_MAX;
    const i16 i16_min = INT16_MIN;
    const i16 arb_to_i16 = cast_f64_to_i16(789.0);
    IHC_CHECK(arb_to_i16 == 789);
    const i16 big_to_i16 = cast_f64_to_i16(f64_max);
    IHC_CHECK(i16_max == big_to_i16);
    const i16 small_to_i16 = cast_f64_to_i16(f64_min);
    IHC_CHECK(i16_min == small_to_i16);

    // f64 to u16
    const u16 u16_max = UINT16_MAX;
    const u16 arb_to_u16 = cast_f64_to_u16(789.0);
    IHC_CHECK(arb_to_u16 == 789);
    const u16 big_to_u16 = cast_f64_to_u16(f64_max);
    IHC_CHECK(u16_max == big_to_u16);
    const u16 small_to_u16 = cast_f64_to_u16(f64_min);
    IHC_CHECK(0u == small_to_u16);

    // f64 to f32
    const f32 f32_max = FLT_MAX;
    const f32 f32_min = -FLT_MAX;
    const f32 arb_to_f32 = cast_f64_to_f32(789.0);
    IHC_CHECK(arb_to_f32 == 789.0f);
    const f32 big_to_f32 = cast_f64_to_f32(f64_max);
    IHC_CHECK(big_to_f32 == f32_max);
    const f32 small_to_f32 = cast_f64_to_f32(f64_min);
    IHC_CHECK(small_to_f32 == f32_min);
}

// Integer bounds rounded to floating-point can land outside the integer range ((f64)UINT64_MAX == 2^64),
// so values at and around 2^63 and 2^64 must be clamped without reaching an out-of-range cast (UB).
IHC_TEST(verify_floating_to_64_bit_integer_boundaries_are_exact)
{
    const f64 two_pow_63 = ldexp(1.0, 63);
    const f64 two_pow_64 = ldexp(1.0, 64);
    const f32 two_pow_63_f = ldexpf(1.0f, 63);
    const f32 two_pow_64_f = ldexpf(1.0f, 64);

    // f64 to u64
    IHC_CHECK(cast_f64_to_u64((f64)UINT64_MAX) == UINT64_MAX); // (f64)UINT64_MAX is 2^64
    IHC_CHECK(cast_f64_to_u64(two_pow_64) == UINT64_MAX);
    IHC_CHECK(cast_f64_to_u64(nextafter(two_pow_64, INFINITY)) == UINT64_MAX);
    IHC_CHECK(cast_f64_to_u64(nextafter(two_pow_64, 0.0)) == UINT64_MAX - 2047u); // 2^64 - 2^11
    IHC_CHECK(cast_f64_to_u64(two_pow_63) == (u64)INT64_MAX + 1u);
    IHC_CHECK(cast_f64_to_u64(nextafter(two_pow_63, INFINITY)) == (u64)INT64_MAX + 1u + 2048u);
    IHC_CHECK(cast_f64_to_u64(nextafter(two_pow_63, 0.0)) == (u64)INT64_MAX - 1023u);
    IHC_CHECK(cast_f64_to_u64(-0.0) == 0u);
    IHC_CHECK(cast_f64_to_u64(nextafter(0.0, -INFINITY)) == 0u);
    IHC_CHECK(cast_f64_to_u64(-1.0) == 0u);
    IHC_CHECK(cast_f64_to_u64(-two_pow_64) == 0u);
    IHC_CHECK(cast_f64_to_u64(NAN) == 0u);
    IHC_CHECK(cast_f64_to_u64(INFINITY) == UINT64_MAX);
    IHC_CHECK(cast_f64_to_u64(-INFINITY) == 0u);

    // f64 to i64
    IHC_CHECK(cast_f64_to_i64((f64)INT64_MAX) == INT64_MAX); // (f64)INT64_MAX is 2^63
    IHC_CHECK(cast_f64_to_i64(two_pow_63) == INT64_MAX);
    IHC_CHECK(cast_f64_to_i64(nextafter(two_pow_63, INFINITY)) == INT64_MAX);
    IHC_CHECK(cast_f64_to_i64(nextafter(two_pow_63, 0.0)) == INT64_MAX - 1023); // 2^63 - 2^10
    IHC_CHECK(cast_f64_to_i64(two_pow_64) == INT64_MAX);
    IHC_CHECK(cast_f64_to_i64((f64)INT64_MIN) == INT64_MIN); // -2^63 is exact
    IHC_CHECK(cast_f64_to_i64(nextafter(-two_pow_63, -INFINITY)) == INT64_MIN);
    IHC_CHECK(cast_f64_to_i64(nextafter(-two_pow_63, 0.0)) == INT64_MIN + 1024);
    IHC_CHECK(cast_f64_to_i64(-two_pow_64) == INT64_MIN);
    IHC_CHECK(cast_f64_to_i64(NAN) == INT64_MIN);
    IHC_CHECK(cast_f64_to_i64(INFINITY) == INT64_MAX);
    IHC_CHECK(cast_f64_to_i64(-INFINITY) == INT64_MIN);

    // f32 to u64
    IHC_CHECK(cast_f32_to_u64(two_pow_64_f) == UINT64_MAX);
    IHC_CHECK(cast_f32_to_u64(nextafterf(two_pow_64_f, INFINITY)) == UINT64_MAX);
    IHC_CHECK(cast_f32_to_u64(nextafterf(two_pow_64_f, 0.0f)) == UINT64_MAX - 1099511627775u); // 2^64 - 2^40
    IHC_CHECK(cast_f32_to_u64(-1.0f) == 0u);
    IHC_CHECK(cast_f32_to_u64(NAN) == 0u);
    IHC_CHECK(cast_f32_to_u64(INFINITY) == UINT64_MAX);
    IHC_CHECK(cast_f32_to_u64(-INFINITY) == 0u);

    // f32 to i64
    IHC_CHECK(cast_f32_to_i64(two_pow_63_f) == INT64_MAX);
    IHC_CHECK(cast_f32_to_i64(nextafterf(two_pow_63_f, INFINITY)) == INT64_MAX);
    IHC_CHECK(cast_f32_to_i64(nextafterf(two_pow_63_f, 0.0f)) == INT64_MAX - 549755813887); // 2^63 - 2^39
    IHC_CHECK(cast_f32_to_i64(-two_pow_63_f) == INT64_MIN);
    IHC_CHECK(cast_f32_to_i64(nextafterf(-two_pow_63_f, -INFINITY)) == INT64_MIN);
    IHC_CHECK(cast_f32_to_i64(NAN) == INT64_MIN);
    IHC_CHECK(cast_f32_to_i64(INFINITY) == INT64_MAX);
    IHC_CHECK(cast_f32_to_i64(-INFINITY) == INT64_MIN);
}

// (f32)INT32_MAX and (f32)UINT32_MAX also round up to 2^31 and 2^32
IHC_TEST(verify_f32_to_32_bit_integer_boundaries_are_exact)
{
    const f32 two_pow_31_f = ldexpf(1.0f, 31);
    const f32 two_pow_32_f = ldexpf(1.0f, 32);

    // f32 to i32
    IHC_CHECK(cast_f32_to_i32(two_pow_31_f) == INT32_MAX);
    IHC_CHECK(cast_f32_to_i32(nextafterf(two_pow_31_f, INFINITY)) == INT32_MAX);
    IHC_CHECK(cast_f32_to_i32(nextafterf(two_pow_31_f, 0.0f)) == INT32_MAX - 127); // 2^31 - 2^7
    IHC_CHECK(cast_f32_to_i32(-two_pow_31_f) == INT32_MIN);
    IHC_CHECK(cast_f32_to_i32(nextafterf(-two_pow_31_f, -INFINITY)) == INT32_MIN);
    IHC_CHECK(cast_f32_to_i32(nextafterf(-two_pow_31_f, 0.0f)) == INT32_MIN + 128);
    IHC_CHECK(cast_f32_to_i32(NAN) == INT32_MIN);
    IHC_CHECK(cast_f32_to_i32(INFINITY) == INT32_MAX);
    IHC_CHECK(cast_f32_to_i32(-INFINITY) == INT32_MIN);

    // f32 to u32
    IHC_CHECK(cast_f32_to_u32(two_pow_32_f) == UINT32_MAX);
    IHC_CHECK(cast_f32_to_u32(nextafterf(two_pow_32_f, INFINITY)) == UINT32_MAX);
    IHC_CHECK(cast_f32_to_u32(nextafterf(two_pow_32_f, 0.0f)) == UINT32_MAX - 255u); // 2^32 - 2^8
    IHC_CHECK(cast_f32_to_u32(-1.0f) == 0u);
    IHC_CHECK(cast_f32_to_u32(NAN) == 0u);
    IHC_CHECK(cast_f32_to_u32(INFINITY) == UINT32_MAX);
    IHC_CHECK(cast_f32_to_u32(-INFINITY) == 0u);
}

// Checks boundaries that are exactly representable in the floating-point type, and their closest neighbours
#define IHC_NUM_CAST_CHECK_EXACT_BOUNDS(from, next, to, to_min, to_max)                \
    IHC_CHECK(cast_##from##_to_##to((from)(to_max)) == (to_max));                      \
    IHC_CHECK(cast_##from##_to_##to(next((from)(to_max), 0)) == (to_max) - 1);         \
    IHC_CHECK(cast_##from##_to_##to(next((from)(to_max), INFINITY)) == (to_max));      \
    IHC_CHECK(cast_##from##_to_##to((from)(to_max) + 1) == (to_max));                  \
    IHC_CHECK(cast_##from##_to_##to((from)(to_min)) == (to_min));                      \
    IHC_CHECK(cast_##from##_to_##to(next((from)(to_min), -INFINITY)) == (to_min));     \
    IHC_CHECK(cast_##from##_to_##to((from)(to_min) - 1) == (to_min));                  \
    IHC_CHECK(cast_##from##_to_##to((from)NAN) == (to_min));                           \
    IHC_CHECK(cast_##from##_to_##to((from)INFINITY) == (to_max));                      \
    IHC_CHECK(cast_##from##_to_##to((from)-INFINITY) == (to_min))

IHC_TEST(verify_floating_to_small_integer_boundaries_are_exact)
{
    IHC_NUM_CAST_CHECK_EXACT_BOUNDS(f64, nextafter,  i8,  INT8_MIN,  INT8_MAX);
    IHC_NUM_CAST_CHECK_EXACT_BOUNDS(f64, nextafter,  i16, INT16_MIN, INT16_MAX);
    IHC_NUM_CAST_CHECK_EXACT_BOUNDS(f64, nextafter,  i32, INT32_MIN, INT32_MAX);
    IHC_NUM_CAST_CHECK_EXACT_BOUNDS(f64, nextafter,  u8,  0,         UINT8_MAX);
    IHC_NUM_CAST_CHECK_EXACT_BOUNDS(f64, nextafter,  u16, 0,         UINT16_MAX);
    IHC_NUM_CAST_CHECK_EXACT_BOUNDS(f64, nextafter,  u32, 0,         UINT32_MAX);

    IHC_NUM_CAST_CHECK_EXACT_BOUNDS(f32, nextafterf, i8,  INT8_MIN,  INT8_MAX);
    IHC_NUM_CAST_CHECK_EXACT_BOUNDS(f32, nextafterf, i16, INT16_MIN, INT16_MAX);
    IHC_NUM_CAST_CHECK_EXACT_BOUNDS(f32, nextafterf, u8,  0,         UINT8_MAX);
    IHC_NUM_CAST_CHECK_EXACT_BOUNDS(f32, nextafterf, u16, 0,         UINT16_MAX);

    // In-range fractions truncate toward zero
    IHC_CHECK(cast_f64_to_i8(-127.9) == -127);
    IHC_CHECK(cast_f64_to_u8(254.9) == 254u);
}

// The assert policy (used when IC_CAST_ASSERT_FUNC is defined) must reject everything the clamp policy clamps
IC_DISABLE_WARNINGS
IHC_TEST(verify_floating_to_integer_assert_policy_rejects_out_of_range)
{
    const f64 two_pow_63 = ldexp(1.0, 63);
    const f64 two_pow_64 = ldexp(1.0, 64);
    const f64 not_a_number = NAN;
    const f64 infinity = INFINITY;

    IHC_CHECK( IC_INNER_SAFE_FLOAT_TO_UNSIGNED_INT(u64, nextafter(two_pow_64, 0.0), 0, UINT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_UNSIGNED_INT(u64, two_pow_64, 0, UINT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_UNSIGNED_INT(u64, (f64)UINT64_MAX, 0, UINT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_UNSIGNED_INT(u64, -1.0, 0, UINT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_UNSIGNED_INT(u64, not_a_number, 0, UINT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_UNSIGNED_INT(u64, infinity, 0, UINT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_UNSIGNED_INT(u64, -infinity, 0, UINT64_MAX));

    IHC_CHECK( IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i64, nextafter(two_pow_63, 0.0), INT64_MIN, INT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i64, two_pow_63, INT64_MIN, INT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i64, (f64)INT64_MAX, INT64_MIN, INT64_MAX));
    IHC_CHECK( IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i64, -two_pow_63, INT64_MIN, INT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i64, nextafter(-two_pow_63, -infinity), INT64_MIN, INT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i64, not_a_number, INT64_MIN, INT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i64, infinity, INT64_MIN, INT64_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i64, -infinity, INT64_MIN, INT64_MAX));

    // Fractions past an exact bound are out of range
    IHC_CHECK( IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i8, 127.0, INT8_MIN, INT8_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i8, 127.5, INT8_MIN, INT8_MAX));
    IHC_CHECK( IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i8, -128.0, INT8_MIN, INT8_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(i8, -128.5, INT8_MIN, INT8_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_UNSIGNED_INT(u8, 255.5, 0, UINT8_MAX));
    IHC_CHECK(!IC_INNER_SAFE_FLOAT_TO_UNSIGNED_INT(u8, -0.5, 0, UINT8_MAX));
}
IC_ENABLE_WARNINGS

#endif // IRON_HAMMER_C_TESTS_NUM_CAST_TEST_H
