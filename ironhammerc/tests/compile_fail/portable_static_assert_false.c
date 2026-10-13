// Compile-fail test (run by CMakeLists.txt at configure time):
// a false portable (fallback) static assert must stop compilation. Its error is a negative
// array size, whose wording differs per compiler, so only the failure itself is checked.
// With IHC_COMPILE_FAIL_CONTROL defined, the condition is true and the file must compile.
#include "ironclib/ic_static_assert.h"

#if defined(IHC_COMPILE_FAIL_CONTROL)
    #define IHC_ASSERTED_CONDITION 1
#else
    #define IHC_ASSERTED_CONDITION 0
#endif

IC_INNER_STATIC_ASSERT_PORTABLE(IHC_ASSERTED_CONDITION, "ihc expected static assert failure");

typedef int ihc_compile_fail_nonempty;
