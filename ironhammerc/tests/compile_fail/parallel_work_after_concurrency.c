// Compile-fail test (run by CMakeLists.txt at configure time):
// including parallel_work.h after a concurrency header already chose the default nullptr panic
// must be a compile error, instead of silently dropping parallel_work.h's logging panic.
// With IHC_COMPILE_FAIL_CONTROL defined, the includes are in the correct order and the file must compile.
#if defined(IHC_COMPILE_FAIL_CONTROL)
    #include "docs/premade/parallel_work.h"
    #include "ironclib/ic_concurrency.h"
#else
    #include "ironclib/ic_concurrency.h"
    #include "docs/premade/parallel_work.h"
#endif

typedef int ihc_compile_fail_nonempty;
