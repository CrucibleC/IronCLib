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

#ifndef IC_DISABLE_WARNINGS_H
#define IC_DISABLE_WARNINGS_H

// This header is used internally within the library.
//
// IC_DISABLE_WARNINGS / IC_ENABLE_WARNINGS wrap macro-generated code (the ic_num_cast.h cast functions),
// where comparisons are often constant or always true/false for a given pair of types by design.
// What is suppressed differs per compiler, on purpose rather than by accident:
// - Clang: everything (-Weverything). Clang has many fine-grained tautology and conversion warnings
//   for this pattern, and listing them all would break as new Clang versions add more.
// - GCC:   only -Wtype-limits, -Wunused and -Wsign-compare, the warnings the generated code triggers
//   under -Wall -Wextra -Wpedantic. Stricter opt-in flags (e.g. -Wconversion, -Wsign-conversion,
//   -Wfloat-conversion, -Wfloat-equal) are NOT suppressed and may need suppressing by the user.
// - MSVC:  C4127 (constant conditional), C4100 / C4189 (unused parameter / variable) and
//   C4056 (floating-point overflow in constant arithmetic), the warnings it gives under /W4.
// - Other compilers: nothing, so the generated code may need manual warning suppression there.

#if defined(_MSC_VER)
// MSVC
#define IC_DISABLE_WARNINGS \
    __pragma(warning(push)) \
    __pragma(warning(disable: 4127))  /* constant conditional */ \
    __pragma(warning(disable: 4100))  /* unused parameter */ \
    __pragma(warning(disable: 4189))  /* unused variable */ \
    __pragma(warning(disable: 4056))  /* FP overflow */
#define IC_ENABLE_WARNINGS \
    __pragma(warning(pop))

// Clang
#elif defined(__clang__)
#define IC_DISABLE_WARNINGS \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Weverything\"")
#define IC_ENABLE_WARNINGS \
    _Pragma("clang diagnostic pop")
    
// GCC
#elif defined(__GNUC__)
#define IC_DISABLE_WARNINGS \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wtype-limits\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused\"") \
    _Pragma("GCC diagnostic ignored \"-Wsign-compare\"")
#define IC_ENABLE_WARNINGS \
    _Pragma("GCC diagnostic pop")
    
// Unknown compiler: do nothing
#else
#define IC_DISABLE_WARNINGS
#define IC_ENABLE_WARNINGS

#endif

#endif // IC_DISABLE_WARNINGS_H
