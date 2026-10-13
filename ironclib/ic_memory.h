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
#include "ic_inline.h"


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


/*
===============================================================================
IC_MALLOC_ARRAY

Purpose:
- Safe array allocation macro
- Handles negative inputs safely
- Prevents overflow in multiplication

Notes:
- Macro is responsible for sanitizing signed input
- Function assumes valid size_t inputs only
- size is evaluated more than once, so avoid side effects (e.g. n++)

Example:
    int *arr = IC_MALLOC_ARRAY(int, n);
===============================================================================
*/

#define IC_MALLOC_ARRAY(type, size) \
    ((type *)ic_inner_malloc_array_impl( \
        ((size) <= 0) ? 0u : (size_t)(size), \
        sizeof(type)))


#endif // IC_MEMORY_H
