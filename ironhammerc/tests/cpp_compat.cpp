// Compiled as C++11 (compile-only, never linked) to cover the C++ branches of IC_STATIC_ASSERT,
// IC_ALIGNAS and IC_ALIGNOF, which no C build selects. Every check happens at compile time.
#include "ironclib/ic_static_assert.h"
#include "ironclib/ic_memory.h"

IC_STATIC_ASSERT(sizeof(char) == 1, "IC_STATIC_ASSERT must work at namespace scope in C++");

struct IhcCppAligned
{
    IC_ALIGNAS(16) ic_byte data[16];
};

IC_STATIC_ASSERT(IC_ALIGNOF(IhcCppAligned) == 16, "IC_ALIGNAS / IC_ALIGNOF must work in C++");
IC_STATIC_ASSERT(IC_ALIGNOF(char) == 1, "IC_ALIGNOF(char) must be 1 in C++");

void ihc_cpp_static_assert_in_function();
void ihc_cpp_static_assert_in_function()
{
    IC_STATIC_ASSERT(sizeof(int) >= 2, "IC_STATIC_ASSERT must work at block scope in C++");
}
