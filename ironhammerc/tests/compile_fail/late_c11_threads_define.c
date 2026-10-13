// Compile-fail test (run by CMakeLists.txt at configure time):
// defining IC_USE_C11_THREADS_AND_ATOMICS after the first concurrency include must be a compile error.
// With IHC_COMPILE_FAIL_CONTROL defined, the late define is skipped and the file must compile.
#undef IC_USE_C11_THREADS_AND_ATOMICS
#include "ironclib/ic_concurrency.h"

#if !defined(IHC_COMPILE_FAIL_CONTROL)
    #define IC_USE_C11_THREADS_AND_ATOMICS
#endif

#include "ironclib/ic_concurrency_signal.h"

typedef int ihc_compile_fail_nonempty;
