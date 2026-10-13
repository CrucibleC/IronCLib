#ifndef IRON_HAMMER_C_TESTS_PORTABILITY_TEST_H
#define IRON_HAMMER_C_TESTS_PORTABILITY_TEST_H

// Tests the portable fallbacks of IC_STATIC_ASSERT and IC_ALIGNOF directly,
// since GCC, Clang and MSVC never select them on their own.

#include "ic_hammer.h"
#include "ironclib/ic_static_assert.h"
#include "ironclib/ic_memory.h"

IC_INNER_STATIC_ASSERT_PORTABLE(sizeof(char) == 1, "portable static assert must work at file scope");
IC_INNER_STATIC_ASSERT_PORTABLE(sizeof(char) == 1, "and may be repeated");

typedef struct portability_test_mixed
{
    char c;
    double d;
    int i;
} portability_test_mixed;

IHC_TEST(verify_portable_static_assert_works_inside_functions)
{
    IC_INNER_STATIC_ASSERT_PORTABLE(2 == 1 + 1, "portable static assert must work at block scope");
    IHC_CHECK(1);
}

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4116) // unnamed type definition in parentheses: exactly what the fallback does
#endif

IHC_TEST(verify_portable_alignof_matches_native_alignof)
{
    IHC_CHECK(IC_INNER_ALIGNOF_PORTABLE(char) == IC_ALIGNOF(char));
    IHC_CHECK(IC_INNER_ALIGNOF_PORTABLE(short) == IC_ALIGNOF(short));
    IHC_CHECK(IC_INNER_ALIGNOF_PORTABLE(int) == IC_ALIGNOF(int));
    IHC_CHECK(IC_INNER_ALIGNOF_PORTABLE(float) == IC_ALIGNOF(float));
    IHC_CHECK(IC_INNER_ALIGNOF_PORTABLE(void*) == IC_ALIGNOF(void*));
    IHC_CHECK(IC_INNER_ALIGNOF_PORTABLE(portability_test_mixed) == IC_ALIGNOF(portability_test_mixed));
    // double and long long are left out: 32-bit x86 System V aligns them to 4 inside structs,
    // but reports 8 for _Alignof, so the two measurements legitimately differ there
}

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#endif // IRON_HAMMER_C_TESTS_PORTABILITY_TEST_H
