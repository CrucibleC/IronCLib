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

/*
===============================================================================
IC CAST SYSTEM
===============================================================================

This header provides a macro-based, overflow-safe numeric casting system
for integer and floating-point conversions up to 64-bit types.

-------------------------------------------------------------------------------
CONFIGURATION (USER OVERRIDABLE)

The following macros may be defined by the user BEFORE including this header:

1. IC_NUM_BITS_PER_BYTE
   - Default: 8
   - Purpose: Defines the number of bits per byte on the target platform.
   - Used for static validation of type size assumptions.

2. IC_CAST_ASSERT_FUNC(expr)
   - Default: undefined, making cast functions clamp casts instead of asserting.
   - Called when a conversion is outside safe bounds before casting.

Example:
    #define IC_CAST_ASSERT_FUNC(expr) assert(expr)

-------------------------------------------------------------------------------
SAFETY MODEL

- All conversions are guaranteed to avoid undefined behavior (UB)
  when used through generated cast functions.
- Overflow and underflow are never permitted in intermediate operations.
- Precision loss and rounding are allowed where required.
- Floating-point inputs safely handle NaN and infinity values.
- All numeric conversions are constrained to types up to 64-bit width.

-------------------------------------------------------------------------------
BASIC USAGE

1. Define your type conversion matrix with the required parameters (type, mold, min, max) - > (type, mold, min, max).
   Mold types: IC_MOLD_SIGNED_INT, IC_MOLD_UNSIGNED_INT, IC_MOLD_FLOAT. 
   Types must be full symbolic values.
   Example:

    #define MY_MATRIX(X) \
        X(int32_t,  SIGNED_INT,   INT32_MIN, INT32_MAX,  uint32_t, UNSIGNED_INT, 0,         UINT32_MAX) \
        X(uint32_t, UNSIGNED_INT, 0,         UINT32_MAX, int32_t,  SIGNED_INT,   INT32_MIN, INT32_MAX)

2. Generate cast functions:

    IC_CASTING_FUNCTIONS(MY_MATRIX)

3. Use generated functions:

    int32_t a = cast_uint32_t_to_int32_t(123u);
    uint32_t b = cast_int32_t_to_uint32_t(-5);

    float f = 123.45f;
    int32_t i = cast_float_to_int32_t(f);

-------------------------------------------------------------------------------
NOTES

- Behavior is deterministic provided IC_CAST_ASSERT_FUNC enforces runtime checks.
- Defining IC_CAST_ASSERT_FUNC disables clamping (unnecessary after an assertion failure).
- IC_CAST_ASSERT_FUNC(expr) can be defined as (void)0 which removes runtime checks completely,
  which can be desirable to for removing all checks and internal casts in performance-critical code after testing. 
- NaN floats get clamped to the minimum value of the target type, and infinities get clamped to the respective min/max.
- Since one of the constraints is clamp Nan and -infinity to the min value, and +infinity to the max value, using 
  cast_float_to_float will actually filter the value, but also means all casts from float lack a compiletime quick path.
===============================================================================
*/

#ifndef IC_NUM_CAST_H
#define IC_NUM_CAST_H

#include "ic_glue_macro.h"
#include "ic_static_assert.h"
#include "ic_inline.h"
#include "ic_disable_warnings.h"
#include <limits.h> // CHAR_BIT, so the bits-per-byte check below always runs

/*
===============================================================================
USER CONFIGURABLE BITS PER BYTE 
===============================================================================
*/

#ifndef IC_NUM_BITS_PER_BYTE
#define IC_NUM_BITS_PER_BYTE 8
#endif

#ifdef CHAR_BIT
IC_STATIC_ASSERT(IC_NUM_BITS_PER_BYTE == CHAR_BIT, "Bits per byte mismatch");
#endif

/*
===============================================================================
TYPE MOLDS
===============================================================================
*/

#define IC_MOLD_SIGNED_INT     SIGNED_INT
#define IC_MOLD_UNSIGNED_INT   UNSIGNED_INT
#define IC_MOLD_FLOAT          FLOAT

/*
===============================================================================
CONVERSION SAFETY CHECKS
===============================================================================
*/

// Every check and clamp honours both bounds (lo, hi), so custom ranges narrower than the type
// (e.g. an unsigned target limited to [10, 100]) clamp correctly at both ends.
// Each clamp branch is cast to to_t on its own so no bound is converted through a mixed common type.

// ---------------------- SIGNED INT -> SIGNED INT ----------------------

// Clamp at compile-time condition
#define IC_COMP_CLAMPABLE_SIGNED_INT_TO_SIGNED_INT(from_min, from_max, to_min, to_max) \
    (((to_min) <= (from_min)) && ((to_max) >= (from_max)))

// Assert at runtime condition
#define IC_INNER_SAFE_SIGNED_INT_TO_SIGNED_INT(to_t, v, lo, hi) \
    ((v) >= (lo) && (v) <= (hi))

// Clamp at runtime (no UB)
#define IC_INNER_CLAMP_SIGNED_INT_TO_SIGNED_INT(to_t, v, lo, hi) \
    ((to_t)((v) < (lo) ? (to_t)(lo) : ((v) > (hi) ? (to_t)(hi) : (to_t)(v))))

// ---------------------- SIGNED INT -> UNSIGNED INT ----------------------

// Clamp at compile-time condition
#define IC_COMP_CLAMPABLE_SIGNED_INT_TO_UNSIGNED_INT(from_min, from_max, to_min, to_max) (0)

// Assert at runtime condition
#define IC_INNER_SAFE_SIGNED_INT_TO_UNSIGNED_INT(to_t, v, lo, hi) \
    ((v) >= 0 && \
     (unsigned long long)(v) >= (unsigned long long)(lo) && \
     (unsigned long long)(v) <= (unsigned long long)(hi))

// Clamp at runtime (no UB)
#define IC_INNER_CLAMP_SIGNED_INT_TO_UNSIGNED_INT(to_t, v, lo, hi) \
    ((to_t)( \
        ((v) < 0 || (unsigned long long)(v) < (unsigned long long)(lo)) ? (to_t)(lo) : \
        ((unsigned long long)(v) > (unsigned long long)(hi)) ? (to_t)(hi) : \
        (to_t)(v) \
    ))

// ---------------------- SIGNED INT -> FLOAT ----------------------

// Clamp at compile-time condition
#define IC_COMP_CLAMPABLE_SIGNED_INT_TO_FLOAT(from_min, from_max, to_min, to_max) \
    ((from_min) >= (to_min) && (from_max) <= (to_max))

// Assert at runtime condition
#define IC_INNER_SAFE_SIGNED_INT_TO_FLOAT(to_t, v, lo, hi) \
    ((double)(v) >= (double)(lo) && (double)(v) <= (double)(hi))

// Clamp at runtime (no UB)
#define IC_INNER_CLAMP_SIGNED_INT_TO_FLOAT(to_t, v, lo, hi) \
    ((to_t)(((double)(v) < (double)(lo)) ? (to_t)(lo) : \
    ((double)(v) > (double)(hi) ? (to_t)(hi) : (to_t)(v))))

// ---------------------- UNSIGNED INT -> SIGNED INT ----------------------
//
// lo and hi are signed and may be negative, so they are only converted to unsigned long long
// after their sign has been checked.

// Clamp at compile-time condition
#define IC_COMP_CLAMPABLE_UNSIGNED_INT_TO_SIGNED_INT(from_min, from_max, to_min, to_max) \
    (((to_min) <= 0 || (unsigned long long)(to_min) <= (unsigned long long)(from_min)) && \
     ((to_max) >= 0 && (unsigned long long)(from_max) <= (unsigned long long)(to_max)))

// Assert at runtime condition
#define IC_INNER_SAFE_UNSIGNED_INT_TO_SIGNED_INT(to_t, v, lo, hi) \
    (((lo) <= 0 || (unsigned long long)(v) >= (unsigned long long)(lo)) && \
     ((hi) >= 0 && (unsigned long long)(v) <= (unsigned long long)(hi)))

// Clamp at runtime (no UB)
#define IC_INNER_CLAMP_UNSIGNED_INT_TO_SIGNED_INT(to_t, v, lo, hi) \
    ((to_t)( \
        ((lo) > 0 && (unsigned long long)(v) < (unsigned long long)(lo)) ? (to_t)(lo) : \
        ((hi) < 0 || (unsigned long long)(v) > (unsigned long long)(hi)) ? (to_t)(hi) : \
        (to_t)(v) \
    ))

// ---------------------- UNSIGNED INT -> UNSIGNED INT ----------------------

// Clamp at compile-time condition
#define IC_COMP_CLAMPABLE_UNSIGNED_INT_TO_UNSIGNED_INT(from_min, from_max, to_min, to_max) \
    (((to_min) <= (from_min)) && ((to_max) >= (from_max)))

// Assert at runtime condition
#define IC_INNER_SAFE_UNSIGNED_INT_TO_UNSIGNED_INT(to_t, v, lo, hi) \
    ((v) >= (lo) && (v) <= (hi))

// Clamp at runtime (no UB)
#define IC_INNER_CLAMP_UNSIGNED_INT_TO_UNSIGNED_INT(to_t, v, lo, hi) \
    ((to_t)((v) < (lo) ? (to_t)(lo) : ((v) > (hi) ? (to_t)(hi) : (to_t)(v))))

// ---------------------- UNSIGNED INT -> FLOAT ----------------------
//
// Compared in double so a floating-point bound is never converted to an integer type
// (e.g. (unsigned long long)FLT_MAX would be UB).

// Clamp at compile-time condition
#define IC_COMP_CLAMPABLE_UNSIGNED_INT_TO_FLOAT(from_min, from_max, to_min, to_max) \
    ((from_min) >= (to_min) && (from_max) <= (to_max))

// Assert at runtime condition
#define IC_INNER_SAFE_UNSIGNED_INT_TO_FLOAT(to_t, v, lo, hi) \
    ((double)(v) >= (double)(lo) && (double)(v) <= (double)(hi))

// Clamp at runtime (no UB)
#define IC_INNER_CLAMP_UNSIGNED_INT_TO_FLOAT(to_t, v, lo, hi) \
    ((to_t)(((double)(v) < (double)(lo)) ? (to_t)(lo) : \
    ((double)(v) > (double)(hi) ? (to_t)(hi) : (to_t)(v))))

// ---------------------- FLOAT -> INT HELPERS ----------------------
//
// Integer bounds such as INT64_MAX or UINT64_MAX are NOT exactly representable as floating-point
// ((double)UINT64_MAX rounds up to 2^64), so comparing against (double)(hi) can accept values that
// are out of range for the target type and make the following cast UB. Instead:
//
//   1. Reject NaN and anything outside the target TYPE's window, using power-of-two bounds which are
//      exact in every binary floating-point format:
//        signed N-bit:   [-2^(N-1), 2^(N-1))
//        unsigned N-bit: [0, 2^N)
//      Inside this window the truncated value always fits in to_t, so (to_t)(v) is well defined.
//   2. Compare the truncated integer t = (to_t)(v) against lo/hi in the integer domain. When t lands
//      exactly on lo or hi, the fractional part decides, which is checked by comparing v with (double)t.
//      (double)t is exact: either v is integral (t == v), or v has a fraction, meaning the format has
//      sub-integer resolution at |v| >= |t|. The source type is at most 64 bits, so it fits in double.
//
// Clamping never converts lo/hi to floating-point (every branch yields to_t), as that can also round
// the bound outside the range of to_t. An unsafe value picks its side as follows:
//   - NaN                        -> lo
//   - outside the type window    -> by sign (the window contains [lo, hi])
//   - inside the type window     -> t <= lo means v is below lo, otherwise v is above hi
//                                   (lo < t < hi would mean lo < v < hi, which is safe)

// Exact 2^n as double, for 1 <= n <= 64
#define IC_INNER_POW2(n) \
    ((double)(1ULL << ((n) - 1)) * 2.0)

#define IC_INNER_BITS(t) \
    (sizeof(t) * IC_NUM_BITS_PER_BYTE)

// Type windows in which (to_t)(v) is well defined (false for NaN)
#define IC_INNER_FLOAT_IN_SIGNED_WINDOW(to_t, v) \
    ((double)(v) >= -IC_INNER_POW2(IC_INNER_BITS(to_t) - 1) && \
     (double)(v) <   IC_INNER_POW2(IC_INNER_BITS(to_t) - 1))

#define IC_INNER_FLOAT_IN_UNSIGNED_WINDOW(to_t, v) \
    ((double)(v) >= 0.0 && \
     (double)(v) <  IC_INNER_POW2(IC_INNER_BITS(to_t)))

// Exact real-number check lo <= v <= hi, only valid when (to_t)(v) is known to be well defined
#define IC_INNER_FLOAT_IN_INT_RANGE(to_t, v, lo, hi) \
    (((to_t)(v) > (lo) || ((to_t)(v) == (lo) && (double)(v) >= (double)(to_t)(v))) && \
     ((to_t)(v) < (hi) || ((to_t)(v) == (hi) && (double)(v) <= (double)(to_t)(v))))

// Bound for an unsafe, non-NaN value, given whether it is inside the type window
#define IC_INNER_FLOAT_OUT_OF_RANGE_BOUND(to_t, v, lo, hi, in_window) \
    ((in_window) ? (((to_t)(v) <= (lo)) ? (to_t)(lo) : (to_t)(hi)) : \
                   (((v) > 0) ? (to_t)(hi) : (to_t)(lo)))

// ---------------------- FLOAT -> SIGNED INT ----------------------

// Clamp at compile-time condition
#define IC_COMP_CLAMPABLE_FLOAT_TO_SIGNED_INT(from_min, from_max, to_min, to_max) (0)

// Assert at runtime condition
#define IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(to_t, v, lo, hi) \
    (IC_INNER_FLOAT_IN_SIGNED_WINDOW(to_t, v) && \
    IC_INNER_FLOAT_IN_INT_RANGE(to_t, v, lo, hi))

// Clamp at runtime (no UB)
#define IC_INNER_CLAMP_FLOAT_TO_SIGNED_INT(to_t, v, lo, hi) \
    ((to_t)(IC_INNER_SAFE_FLOAT_TO_SIGNED_INT(to_t, v, lo, hi) ? (to_t)(v) : \
    ((v) != (v)) ? (to_t)(lo) : \
    IC_INNER_FLOAT_OUT_OF_RANGE_BOUND(to_t, v, lo, hi, IC_INNER_FLOAT_IN_SIGNED_WINDOW(to_t, v))))


// ---------------------- FLOAT -> UNSIGNED INT ----------------------

// Clamp at compile-time condition
#define IC_COMP_CLAMPABLE_FLOAT_TO_UNSIGNED_INT(from_min, from_max, to_min, to_max) (0)

// Assert at runtime condition
#define IC_INNER_SAFE_FLOAT_TO_UNSIGNED_INT(to_t, v, lo, hi) \
    (IC_INNER_FLOAT_IN_UNSIGNED_WINDOW(to_t, v) && \
    IC_INNER_FLOAT_IN_INT_RANGE(to_t, v, lo, hi))

// Clamp at runtime (no UB)
#define IC_INNER_CLAMP_FLOAT_TO_UNSIGNED_INT(to_t, v, lo, hi) \
    ((to_t)(IC_INNER_SAFE_FLOAT_TO_UNSIGNED_INT(to_t, v, lo, hi) ? (to_t)(v) : \
    ((v) != (v)) ? (to_t)(lo) : \
    IC_INNER_FLOAT_OUT_OF_RANGE_BOUND(to_t, v, lo, hi, IC_INNER_FLOAT_IN_UNSIGNED_WINDOW(to_t, v))))


// ---------------------- FLOAT -> FLOAT ---------------------- 

// Clamp at compile-time condition
#define IC_COMP_CLAMPABLE_FLOAT_TO_FLOAT(from_min, from_max, to_min, to_max) \
    0 // ((to_min <= from_min) && (to_max >= from_max)) cannot be used if NaN and infinity shall be clamped

// Assert at runtime condition
#define IC_INNER_SAFE_FLOAT_TO_FLOAT(to_t, v, lo, hi) \
    ((v) == (v) && (v) >= (lo) && (v) <= (hi))

// Clamp at runtime (no UB)
#define IC_INNER_CLAMP_FLOAT_TO_FLOAT(to_t, v, lo, hi) \
    ((to_t)((v) != (v) ? (lo) : \
    ((v) < (lo) ? (lo) : \
    ((v) > (hi) ? (hi) : (v)))))

/*
===============================================================================
CONVERSION DISPATCHERS
===============================================================================
*/

#define IC_INNER_POLICY(from_mold, to_mold) \
    IC_GLUE4(IC_INNER_SAFE_, from_mold, _TO_, to_mold)

#define IC_COMP_CLAMPABLE(from_mold, to_mold) \
    IC_GLUE4(IC_COMP_CLAMPABLE_, from_mold, _TO_, to_mold)

#define IC_INNER_CLAMP_DISPATCH(from_mold, to_mold) \
    IC_GLUE4(IC_INNER_CLAMP_, from_mold, _TO_, to_mold)

/*
===============================================================================
CAST NAME
===============================================================================
*/

#define IC_INNER_CAST_NAME(from, to) \
    IC_GLUE4(cast_, from, _to_, to)

/*
===============================================================================
CAST GENERATOR
===============================================================================
*/

#ifndef IC_CAST_ASSERT_FUNC // If not using assertions, generate clamp cast functions

#define IC_INNER_GEN_CAST(from_t, from_mold, from_min, from_max, \
                          to_t, to_mold, to_min, to_max) \
IC_HEADER_FUNC to_t \
IC_INNER_CAST_NAME(from_t, to_t)(const from_t v) \
{ \
    IC_STATIC_ASSERT(sizeof(from_t) * IC_NUM_BITS_PER_BYTE <= 64, "From type " #from_t " is bigger than 64 bits"); \
    IC_STATIC_ASSERT(sizeof(to_t) * IC_NUM_BITS_PER_BYTE   <= 64, "To type " #to_t " is bigger than 64 bits"); \
    \
    if (IC_COMP_CLAMPABLE(from_mold, to_mold)(from_min, from_max, to_min, to_max)) { \
        return (to_t)v; \
    } \
    return IC_INNER_CLAMP_DISPATCH(from_mold, to_mold)(to_t, v, to_min, to_max); \
}

#else // If assertion function is defined, generate assert-based casts instead of clamping

#define IC_INNER_GEN_CAST(from_t, from_mold, from_min, from_max, \
                          to_t, to_mold, to_min, to_max) \
IC_HEADER_FUNC to_t \
IC_INNER_CAST_NAME(from_t, to_t)(const from_t v) \
{ \
    IC_STATIC_ASSERT(sizeof(from_t) * IC_NUM_BITS_PER_BYTE <= 64, "From type " #from_t " is bigger than 64 bits"); \
    IC_STATIC_ASSERT(sizeof(to_t) * IC_NUM_BITS_PER_BYTE   <= 64, "To type " #to_t " is bigger than 64 bits"); \
    \
    if (IC_COMP_CLAMPABLE(from_mold, to_mold)(from_min, from_max, to_min, to_max)) { \
        return (to_t)v; \
    } \
    IC_CAST_ASSERT_FUNC(IC_INNER_POLICY(from_mold, to_mold)(to_t, v, to_min, to_max)); \
    return (to_t)(v); \
}

#endif


/*
===============================================================================
PUBLIC API
===============================================================================
*/

#define IC_CASTING_FUNCTIONS(TYPE_MATRIX) \
    IC_DISABLE_WARNINGS \
    TYPE_MATRIX(IC_INNER_GEN_CAST) \
    IC_ENABLE_WARNINGS

#endif // IC_NUM_CAST_H
