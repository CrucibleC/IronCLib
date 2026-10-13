/*
 * Copyright 2026 CrucibleC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IC_MEMORY_H
#define IC_MEMORY_H

/*
===============================================================================
IC Memory Abstraction Layer

C Compatibility:
- C99+ optional features supported
- MSVC / GCC / Clang supported

Provides:
    ic_byte
    IC_ALIGNAS
    IC_ALIGNAS_TYPE
    IC_ALIGNOF
    IC_MALLOC_ARRAY

===============================================================================
*/

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h> // SIZE_MAX
#include <limits.h> // ULLONG_MAX
#include "ic_inline.h"

// C++ compatibility: C linkage for everything declared below. Placed after the #includes,
// since system and other library headers must not be wrapped in an extern "C" block.
#ifdef __cplusplus
extern "C" {
#endif


/*
-------------------------------------------------------------------------------
Basic byte type
-------------------------------------------------------------------------------
*/

typedef unsigned char ic_byte;


/*
===============================================================================
Alignment support
===============================================================================
*/

// Portable alignment measurement for C (not C++, which forbids defining a type inside sizeof).
// Always defined so it can be tested on compilers that never select the fallback below.
// In struct { char c; T m; }, common ABIs place m at offset alignof(T) and end the struct right after it
// (sizeof(T) is a multiple of alignof(T)), so the size difference is exactly alignof(T).
// This avoids defining a type inside offsetof, which C23 makes undefined behavior.
#define IC_INNER_ALIGNOF_PORTABLE(x) (sizeof(struct { ic_byte c; x member; }) - sizeof(x))

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L

    #define IC_ALIGNAS(x) _Alignas(x)
    #define IC_ALIGNOF(x) _Alignof(x)

#elif defined(__cplusplus) && __cplusplus >= 201103L

    #define IC_ALIGNAS(x) alignas(x)
    #define IC_ALIGNOF(x) alignof(x)

#elif defined(_MSC_VER)

    #define IC_ALIGNAS(x) __declspec(align(x))
    #define IC_ALIGNOF(x) __alignof(x)

#elif defined(__GNUC__) || defined(__clang__)

    #define IC_ALIGNAS(x) __attribute__((aligned(x)))
    #define IC_ALIGNOF(x) __alignof__(x)

#elif defined(__cplusplus)

    #error "IC_ALIGNOF requires C++11, or a compiler with alignment extensions (MSVC / GCC / Clang)"

#else

    // Pre-C11 C on an unknown compiler: no way to force alignment, but it can still be measured
    #define IC_ALIGNAS_IS_BLANK
    #define IC_ALIGNAS(x)
    #define IC_ALIGNOF(x) IC_INNER_ALIGNOF_PORTABLE(x)

#endif

#define IC_ALIGNAS_TYPE(type) IC_ALIGNAS(IC_ALIGNOF(type))


/*
===============================================================================
Internal allocation helper
===============================================================================

Purpose:
- Performs safe multiplication of (count * elem_size)
- Prevents integer overflow before calling malloc
- Operates only on size_t

Safety rules:
- count == 0 or elem_size == 0 -> NULL
- overflow -> NULL
===============================================================================
*/

IC_HEADER_FUNC void* ic_inner_malloc_array_impl(const size_t count, const size_t elem_size)
{
    if (count == 0 || elem_size == 0) {
        return NULL;
    }

    // Overflow check
    if (count > ((size_t)-1) / elem_size) {
        return NULL;
    }

    return malloc(count * elem_size);
}

// Typed entry points for the C11 IC_MALLOC_ARRAY: every integer type is converted to the widest
// type of the same signedness, so no value changes, and a count that does not fit size_t is rejected.
IC_HEADER_FUNC void* ic_inner_malloc_array_unsigned(const unsigned long long count, const size_t elem_size)
{
#if SIZE_MAX < ULLONG_MAX
    if (count > (unsigned long long)SIZE_MAX) {
        return NULL;
    }
#endif

    return ic_inner_malloc_array_impl((size_t)count, elem_size);
}

IC_HEADER_FUNC void* ic_inner_malloc_array_signed(const long long count, const size_t elem_size)
{
    if (count <= 0) {
        return NULL;
    }

    return ic_inner_malloc_array_unsigned((unsigned long long)count, elem_size);
}


/*
===============================================================================
IC_MALLOC_ARRAY

Purpose:
- Safe array allocation macro
- Handles negative inputs safely
- Prevents overflow in multiplication

Implementations:
- C11 and newer (GCC, Clang, MSVC /std:c11 and any other C11 compiler):
    size is evaluated exactly once, so side effects such as n++ are safe.
    _Generic picks a helper by the integer type of size (the controlling expression of
    _Generic is never evaluated), and size is then evaluated once as the call argument.
    size must have an integer type: anything else (e.g. a float) is a compile error.
- Fallback (C99, and C++ where _Generic does not exist):
    size is evaluated MORE THAN ONCE, so avoid side effects (e.g. n++).
    Any arithmetic type is accepted and converted to size_t after the negative check.
IC_MALLOC_ARRAY_SINGLE_EVALUATION is 1 for the C11 version and 0 for the fallback.

Example:
    int *arr = IC_MALLOC_ARRAY(int, n);
===============================================================================
*/

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L

    #define IC_MALLOC_ARRAY_SINGLE_EVALUATION 1

    // Plain char may be signed or unsigned; the signed helper handles both ranges correctly
    #define IC_MALLOC_ARRAY(type, size) \
        ((type *)_Generic((size), \
            char:               ic_inner_malloc_array_signed, \
            signed char:        ic_inner_malloc_array_signed, \
            short:              ic_inner_malloc_array_signed, \
            int:                ic_inner_malloc_array_signed, \
            long:               ic_inner_malloc_array_signed, \
            long long:          ic_inner_malloc_array_signed, \
            _Bool:              ic_inner_malloc_array_unsigned, \
            unsigned char:      ic_inner_malloc_array_unsigned, \
            unsigned short:     ic_inner_malloc_array_unsigned, \
            unsigned int:       ic_inner_malloc_array_unsigned, \
            unsigned long:      ic_inner_malloc_array_unsigned, \
            unsigned long long: ic_inner_malloc_array_unsigned \
        )((size), sizeof(type)))

#else

    #define IC_MALLOC_ARRAY_SINGLE_EVALUATION 0

    // Fallback: evaluates size more than once (see the documentation above)
    #define IC_MALLOC_ARRAY(type, size) \
        ((type *)ic_inner_malloc_array_impl( \
            ((size) <= 0) ? 0u : (size_t)(size), \
            sizeof(type)))

#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif // IC_MEMORY_H
