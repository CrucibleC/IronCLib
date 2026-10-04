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

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L

    #define IC_ALIGNAS(x) _Alignas(x)
    #define IC_ALIGNOF(x) _Alignof(x)

#elif defined(_MSC_VER)

    #define IC_ALIGNAS(x) __declspec(align(x))
    #define IC_ALIGNOF(x) __alignof(x)

#elif defined(__GNUC__) || defined(__clang__)

    #define IC_ALIGNAS(x) __attribute__((aligned(x)))
    #define IC_ALIGNOF(x) __alignof__(x)

#else

    #define IC_ALIGNAS_IS_BLANK
    #define IC_ALIGNAS(x)
    #define IC_ALIGNOF(x) (offsetof(struct { ic_byte c; x member; }, member))

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

Example:
    int *arr = IC_MALLOC_ARRAY(int, n);
===============================================================================
*/

#define IC_MALLOC_ARRAY(type, size) \
    ((type *)ic_inner_malloc_array_impl( \
        ((size) <= 0) ? 0u : (size_t)(size), \
        sizeof(type)))


#endif // IC_MEMORY_H