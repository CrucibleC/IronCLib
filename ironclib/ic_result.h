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

#ifndef IC_RESULT_H
#define IC_RESULT_H

/*
===============================================================================
IC RESULT

C Compatibility:
- C99+: supported
- C11+: recommended
- GCC / Clang / MSVC supported

Dependencies:
- ic_inline.h        -> IC_HEADER_FUNC

Design:
- Result<T,E> type
- automatic constructor generation
- TRY propagation that is type-safe via assignment rules

-------------------------------------------------------------------------------

Example Usage:

    #include <stdio.h>
    #include "ic_result.h"

    // Define a Result type: char value, int error
    IC_RESULT_TYPE(CharResult, char, int)

    CharResult get_letter(int ok) {
        if (ok) {
            return CharResult_ok('A');
        } else {
            return CharResult_err(-1);
        }
    }
    int main() {
        CharResult r = get_letter(1);
        if (IC_RESULT_IS_OK(r)) {
            printf("%c\n", IC_RESULT_VALUE(r));
        } else {
            printf("Error: %d\n", IC_RESULT_ERROR(r));
        }
        return 0;
    }

===============================================================================
*/

#include "ic_inline.h"

/*
==============================================================================
  RESULT TYPE + AUTOMATIC CONSTRUCTORS

This macro generates:

1. Result struct type
2. ok constructor
3. error constructor

==============================================================================
*/

#define IC_RESULT_TYPE(name, value_type, error_type) \
    typedef struct { \
        int ok; \
        union { \
            value_type value; \
            error_type error; \
        } data; \
    } name; \
    \
    IC_HEADER_FUNC name name##_ok(value_type v) { \
        name r; \
        r.ok = 1; \
        r.data.value = v; \
        return r; \
    } \
    \
    IC_HEADER_FUNC name name##_err(error_type e) { \
        name r; \
        r.ok = 0; \
        r.data.error = e; \
        return r; \
    }


/*
==============================================================================
  ACCESS HELPERS
==============================================================================
*/

#define IC_RESULT_IS_OK(r) ((r).ok)
#define IC_RESULT_VALUE(r) ((r).data.value)
#define IC_RESULT_ERROR(r) ((r).data.error)


/*
==============================================================================
  TRY PROPAGATION

- early return on error
- zero runtime overhead in optimized builds

Implementations:
- Compilers with typeof (C23 typeof, GCC / Clang __typeof__ in every mode, MSVC __typeof__
  in C mode since Visual Studio 2022 17.9, and decltype in C++11):
    result is evaluated exactly once into a temporary, so passing a function call is safe,
    e.g. IC_TRY_RETURN_ERR_AS(IntResult, parse(text));
    The temporary is named ic_inner_try_result, so result must not refer to a variable of that name.
- Fallback (other compilers, older MSVC, and C++ before C++11):
    result is evaluated once when it is ok, but TWICE when it is an error, so pass a variable,
    not a function call or anything with side effects.
IC_TRY_SINGLE_EVALUATION is 1 for the typeof version and 0 for the fallback.

==============================================================================
*/

#if !defined(__cplusplus) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define IC_INNER_TYPEOF(x) typeof(x)
#elif !defined(__cplusplus) && (defined(__GNUC__) || defined(__clang__))
    #define IC_INNER_TYPEOF(x) __typeof__(x) // accepted in strict ISO modes, unlike plain typeof
#elif !defined(__cplusplus) && defined(_MSC_VER) && _MSC_VER >= 1939
    #define IC_INNER_TYPEOF(x) __typeof__(x)
#elif defined(__cplusplus) && (__cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1600))
    // decltype of an lvalue expression is a reference, which binds to it without a copy:
    // still exactly one evaluation. (MSVC reports __cplusplus as 199711L by default.)
    #define IC_INNER_TYPEOF(x) decltype(x)
#endif

#if defined(IC_INNER_TYPEOF)

    #define IC_TRY_SINGLE_EVALUATION 1

    #define IC_TRY_RETURN_ERR_AS(return_type, result) \
        do { \
            IC_INNER_TYPEOF(result) ic_inner_try_result = (result); \
            if (!ic_inner_try_result.ok) { \
                return return_type##_err(ic_inner_try_result.data.error); \
            } \
        } while (0)

#else

    #define IC_TRY_SINGLE_EVALUATION 0

    // Fallback: evaluates result twice on the error path (see the documentation above)
    #define IC_TRY_RETURN_ERR_AS(return_type, result) \
        do { \
            if (!(result).ok) { \
                return return_type##_err((result).data.error); \
            } \
        } while (0)

#endif


#endif // IC_RESULT_H
