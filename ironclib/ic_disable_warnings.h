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

#if defined(_MSC_VER)
// MSVC
#define IC_DISABLE_WARNINGS \
    __pragma(warning(push)) \
    __pragma(warning(disable: 4127))  /* constant conditional */ \
    __pragma(warning(disable: 4100))  /* unused parameter */ \
    __pragma(warning(disable: 4189))  /* unused variable */ \
    __pragma(warning(disable: 4056))  /* FP overflow */
#define IC_ENABLE_WARNINGS \
    __pragma(warning(pop));

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
