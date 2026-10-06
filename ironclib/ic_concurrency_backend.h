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

#ifndef IC_CONCURRENCY_BACKEND_H
#define IC_CONCURRENCY_BACKEND_H

// This header is used internally within the library.
// Due to C11 threading not being fully supported across all platforms and compilers, it must 
// be manually actived by defining IC_USE_C11_THREADS_AND_ATOMICS before including the header.

#if defined(IC_USE_C11_THREADS_AND_ATOMICS)
    #if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
        #include <threads.h>
        #define IC_THREAD_C11
    #else
        #error "IC_CONCURRENCY ERROR: IC_USE_C11_THREADS_AND_ATOMICS requires a C11 compiler."
    #endif
#elif defined(_WIN32)
    #define IC_THREAD_WIN
    #include <Windows.h>
#elif defined(__unix__) || defined(__APPLE__)
    // In strict ISO mode (-std=c99/c11) glibc hides POSIX functions such as nanosleep.
    // Compile with _POSIX_C_SOURCE=200809L (or higher) defined before any system header.
    #define IC_THREAD_PTHREAD
    #include <pthread.h>
#else
    #error "IC_CONCURRENCY ERROR: No supported threading backend (Win / pthread / optional C11)."
#endif

#if defined(IC_USE_C11_THREADS_AND_ATOMICS) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(_MSC_VER)
    #define IC_ATOMIC_C11
    #include <stdatomic.h>
#elif defined(__GNUC__) || defined(__clang__)
    #define IC_ATOMIC_GCC
#elif defined(_MSC_VER)
    #define IC_ATOMIC_MSVC
    #include <Windows.h>
#else
    #error "IC_CONCURRENCY ERROR: No supported atomic backend (C11 atomics / GCC built-ins / MSVC Interlocked)."
#endif

#endif // IC_CONCURRENCY_BACKEND_H