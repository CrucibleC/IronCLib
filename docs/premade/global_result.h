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

#ifndef PREMADE_GLOBAL_RESULT_H
#define PREMADE_GLOBAL_RESULT_H

#include "ironclib/ic_result.h"
#include "global_error.h"
#include <stddef.h>
#include <stdbool.h>
// #include "numbers.h" // Uncomment if you want to use the predefined number types in your result values

/*
USAGE:
    - Include this header in your application to define a global Result type using the global error type.
    - Use GLOBAL_RESULT_TYPE(Type) to define Result types for your specific value types.
    - Define result types next to struct definitions for better organization, unless a common type, then extend this file.
    - Example: if (result.ok) { foo(result.data.value); } else { bar(result.data.error); }

INCLUDED RESULT TYPES (all using global AppError type):
      RESULT TYPE       VALUE TYPE      OK CONSTRUCTOR EXAMPLE                  ERR CONSTRUCTOR EXAMPLE
      ---------------------------------------------------------------------------------------------------------------------
    - CharResult:       char,           CharResult_ok('A'),                     CharResult_err(AppError_Runtime)
    - IntResult:        int,            IntResult_ok(42),                       IntResult_err(AppError_Runtime)
    - FloatResult:      float,          FloatResult_ok(3.14f),                  FloatResult_err(AppError_Runtime)
    - DoubleResult:     double,         DoubleResult_ok(2.718),                 DoubleResult_err(AppError_Runtime)
    - UIntResult:       unsigned int,   UIntResult_ok(100),                     UIntResult_err(AppError_Runtime)
    - StringViewResult: const char*,    StringViewResult_ok("Hello, World!"),   StringViewResult_err(AppError_Runtime)
    - VoidPtrResult:    void*,          VoidPtrResult_ok(some_memory),          VoidPtrResult_err(AppError_Runtime)
    - SizeResult:       size_t,         SizeResult_ok(1024),                    SizeResult_err(AppError_Runtime)
    - BoolResult:       bool,           BoolResult_ok(true),                    BoolResult_err(AppError_Runtime)
    - VoidResult:       VoidType,       VoidResult_ok(),                        VoidResult_err(AppError_Runtime)

IF numbers.h IS INCLUDED:
    - I8Result (i8), I16Result (i16), I32Result (i32), I64Result (i64)
    - U8Result (u8), U16Result (u16), U32Result (u32), U64Result (u64)
    - F32Result (f32), F64Result (f64)
*/

// Define a macro to generate standard Result types for the application using the global error type
#define INNER_RESULT_NAME_IMPL(Type) Type##Result
#define INNER_RESULT_NAME(Type) INNER_RESULT_NAME_IMPL(Type)
#define INNER_GLOBAL_RESULT_TYPE_IMPL(Name, Type) IC_RESULT_TYPE(Name, Type, AppError)

// API
#define GLOBAL_RESULT_TYPE(Type) INNER_GLOBAL_RESULT_TYPE_IMPL(INNER_RESULT_NAME(Type), Type)

// Common result types
IC_RESULT_TYPE(CharResult, char, AppError)
IC_RESULT_TYPE(IntResult, int, AppError)
IC_RESULT_TYPE(FloatResult, float, AppError)
IC_RESULT_TYPE(DoubleResult, double, AppError)
IC_RESULT_TYPE(UIntResult, unsigned int, AppError)
typedef const char* StringView;
GLOBAL_RESULT_TYPE(StringView)
typedef void* VoidPtr;
GLOBAL_RESULT_TYPE(VoidPtr)
IC_RESULT_TYPE(SizeResult, size_t, AppError)
IC_RESULT_TYPE(BoolResult, bool, AppError)

// VoidResult
typedef struct { char _; } VoidType;
GLOBAL_RESULT_TYPE(VoidType)
typedef VoidTypeResult VoidResult;
#define VoidResult_ok() VoidTypeResult_ok((VoidType){0})
#define VoidResult_err(x) VoidTypeResult_err(x)

// If numbers.h is included, number result types are also generated
#ifdef PREMADE_NUMBERS_H
IC_RESULT_TYPE(I8Result, i8, AppError)
IC_RESULT_TYPE(I16Result, i16, AppError)
IC_RESULT_TYPE(I32Result, i32, AppError)
IC_RESULT_TYPE(I64Result, i64, AppError)
IC_RESULT_TYPE(U8Result, u8, AppError)
IC_RESULT_TYPE(U16Result, u16, AppError)
IC_RESULT_TYPE(U32Result, u32, AppError)
IC_RESULT_TYPE(U64Result, u64, AppError)
IC_RESULT_TYPE(F32Result, f32, AppError)
IC_RESULT_TYPE(F64Result, f64, AppError)
#endif

#endif // PREMADE_GLOBAL_RESULT_H