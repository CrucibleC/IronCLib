// Compiled as its own translation unit with ic_num_cast.h as the very first include,
// so the build fails if its macros need anything the header does not include itself.
// (<stdint.h> is only needed for the user's own matrix bounds below, not by the header.)
#include "ironclib/ic_num_cast.h"
#include <stdint.h>

typedef int32_t standalone_i32;
typedef uint8_t standalone_u8;
typedef double  standalone_f64;

#define STANDALONE_CAST_MATRIX(X) \
    X(standalone_i32, IC_MOLD_SIGNED_INT, INT32_MIN, INT32_MAX, standalone_u8,  IC_MOLD_UNSIGNED_INT, 0,         UINT8_MAX) \
    X(standalone_f64, IC_MOLD_FLOAT,      -1e300,    1e300,     standalone_i32, IC_MOLD_SIGNED_INT,   INT32_MIN, INT32_MAX)

IC_CASTING_FUNCTIONS(STANDALONE_CAST_MATRIX)

int standalone_num_cast_value(void);
int standalone_num_cast_value(void)
{
    return (int)cast_standalone_i32_to_standalone_u8(300) + (int)cast_standalone_f64_to_standalone_i32(1.5);
}
