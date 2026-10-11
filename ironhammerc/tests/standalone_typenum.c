// Compiled as its own translation unit with ic_typenum.h as the very first include,
// so the build fails if the header stops being self-contained (works on its own).
#include "ironclib/ic_typenum.h"

#define STANDALONE_TYPENUM_LIST(X, Type) \
    X(Type, First, 0, "First value") \
    X(Type, Second, 1, "Second value")

IC_TYPENUM_FULL(StandaloneTypenum, int, STANDALONE_TYPENUM_LIST)

int standalone_typenum_second_value(void);
int standalone_typenum_second_value(void)
{
    // Uses everything generated, as unused static functions/consts warn when defined in a source file
    const int is_named = (StandaloneTypenum_to_string(StandaloneTypenum_First)[0] == 'F');
    const int is_distinct = !StandaloneTypenum_eq(StandaloneTypenum_First, StandaloneTypenum_Second);
    return (is_named && is_distinct) ? StandaloneTypenum_get(StandaloneTypenum_Second) : -1;
}
