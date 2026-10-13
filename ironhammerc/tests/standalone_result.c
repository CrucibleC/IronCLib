// Compiled as its own translation unit with ic_result.h as the very first include,
// so the build fails if its macros need anything the header does not include itself.
#include "ironclib/ic_result.h"

IC_RESULT_TYPE(StandaloneResult, int, int)

static StandaloneResult standalone_result_increment(const StandaloneResult in)
{
    IC_TRY_RETURN_ERR_AS(StandaloneResult, in);
    return StandaloneResult_ok(IC_RESULT_VALUE(in) + 1);
}

int standalone_result_value(void);
int standalone_result_value(void)
{
    const StandaloneResult ok = standalone_result_increment(StandaloneResult_ok(1));
    const StandaloneResult err = standalone_result_increment(StandaloneResult_err(5));
    return (IC_RESULT_IS_OK(ok) && !IC_RESULT_IS_OK(err)) ? IC_RESULT_VALUE(ok) + IC_RESULT_ERROR(err) : -1;
}
