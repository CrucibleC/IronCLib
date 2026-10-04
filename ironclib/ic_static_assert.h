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

#ifndef IC_STATIC_ASSERT_H
#define IC_STATIC_ASSERT_H

/*
===============================================================================
IC Static Assert

C Version Compatibility:
- C11+: uses _Static_assert
- MSVC: uses static_assert
- Pre-C11: fallback typedef trick

Usage:
    IC_STATIC_ASSERT(sizeof(int) == 4, "int must be 4 bytes");

For compilers without native static assert support, this will cause a compile-time error if the condition is false, but the error message may be less clear.
===============================================================================
*/

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L

    #define IC_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

#elif defined(_MSC_VER) && _MSC_VER >= 1600

    #define IC_STATIC_ASSERT(cond, msg) static_assert(cond, msg)

#else

    #if defined(__COUNTER__)
        #define IC_INTERNAL_SA_UNIQUE_ID __COUNTER__
    #else
        #define IC_INTERNAL_SA_UNIQUE_ID __LINE__
    #endif

    #define IC_STATIC_ASSERT(cond, msg) \
        typedef char IC_GLUE(static_assert_failed_, IC_INTERNAL_SA_UNIQUE_ID) \
        [(cond) ? 1 : -1]

#endif

#endif // IC_STATIC_ASSERT_H