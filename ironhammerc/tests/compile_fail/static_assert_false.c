// Compile-fail test (run by CMakeLists.txt at configure time):
// a false IC_STATIC_ASSERT must stop compilation and show its message.
// With IHC_COMPILE_FAIL_CONTROL defined, the condition is true and the file must compile.
#include "ironclib/ic_static_assert.h"

#if defined(IHC_COMPILE_FAIL_CONTROL)
    #define IHC_ASSERTED_CONDITION 1
#else
    #define IHC_ASSERTED_CONDITION 0
#endif

IC_STATIC_ASSERT(IHC_ASSERTED_CONDITION, "ihc expected static assert failure");

typedef int ihc_compile_fail_nonempty;
