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
- C++11+ (and MSVC C++ since VS2010): uses static_assert
- Pre-C11 C (including MSVC without /std:c11): fallback extern array trick

Usage:
    IC_STATIC_ASSERT(sizeof(int) == 4, "int must be 4 bytes");

For compilers without native static assert support, this will cause a compile-time error if the condition is false, but the error message may be less clear.
===============================================================================
*/

// Portable fallback, always defined so it can be tested on compilers that never select it.
// A negative array size fails to compile. Redeclaring the same extern array is legal,
// so no unique name is needed (which would require __COUNTER__, a C2y extension).
#if defined(__GNUC__)
    #define IC_INTERNAL_SA_UNUSED __attribute__((unused)) // gcc warns inside functions otherwise
#else
    #define IC_INTERNAL_SA_UNUSED
#endif

#define IC_INNER_STATIC_ASSERT_PORTABLE(cond, msg) \
    extern char ic_static_assert_failed[(cond) ? 1 : -1] IC_INTERNAL_SA_UNUSED

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L

    #define IC_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

#elif defined(__cplusplus) && (__cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1600))

    // static_assert is a C++ keyword, but not available in C before C11 (MSVC included).
    // MSVC reports __cplusplus as 199711L unless /Zc:__cplusplus is set, hence the _MSC_VER check.
    #define IC_STATIC_ASSERT(cond, msg) static_assert(cond, msg)

#else

    #define IC_STATIC_ASSERT(cond, msg) IC_INNER_STATIC_ASSERT_PORTABLE(cond, msg)

#endif

#endif // IC_STATIC_ASSERT_H
