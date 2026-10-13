// Compiled as C++11 (compile-only, never linked) to check that every IronCLib header can be used from C++:
// - the extern "C" guards and C++-valid code in all library and premade headers,
// - the C++ branches of IC_STATIC_ASSERT, IC_ALIGNAS, IC_ALIGNOF and IC_TRY_RETURN_ERR_AS (decltype),
//   which no C build selects.

// parallel_work.h picks the nullptr panic, so it must come before the other concurrency headers
#include "docs/premade/parallel_work.h"
#include "docs/premade/numbers.h"
#include "docs/premade/global_result.h"
#include "ironclib/ic.h"

// ------------------------------------------------------------------
// Static assert and alignment
// ------------------------------------------------------------------

IC_STATIC_ASSERT(sizeof(char) == 1, "IC_STATIC_ASSERT must work at namespace scope in C++");

struct IhcCppAligned
{
    IC_ALIGNAS(16) ic_byte data[16];
};

IC_STATIC_ASSERT(IC_ALIGNOF(IhcCppAligned) == 16, "IC_ALIGNAS / IC_ALIGNOF must work in C++");
IC_STATIC_ASSERT(IC_ALIGNOF(char) == 1, "IC_ALIGNOF(char) must be 1 in C++");

IC_OPAQUE_STORAGE(IhcCppOpaque, IC_ALIGNOF(int), sizeof(int) * 2)
IC_STATIC_ASSERT(sizeof(IhcCppOpaque) == sizeof(int) * 2, "IC_OPAQUE_STORAGE must work in C++");

IC_STATIC_ASSERT(IC_TRY_SINGLE_EVALUATION == 1, "C++11 must use the decltype version of IC_TRY_RETURN_ERR_AS");

void ihc_cpp_static_assert_in_function();
void ihc_cpp_static_assert_in_function()
{
    IC_STATIC_ASSERT(sizeof(int) >= 2, "IC_STATIC_ASSERT must work at block scope in C++");
}

// ------------------------------------------------------------------
// Using the headers (functions with external linkage, so none is "unused")
// ------------------------------------------------------------------

IntResult ihc_cpp_try_propagates(const FloatResult in);
IntResult ihc_cpp_try_propagates(const FloatResult in)
{
    IC_TRY_RETURN_ERR_AS(IntResult, in);
    return IntResult_ok(1);
}

static void ihc_cpp_co_step(void* data)
{
    int* const count = static_cast<int*>(data);
    (*count)++;
}

int ihc_cpp_use_headers();
int ihc_cpp_use_headers()
{
    // ic_memory.h
    int* const values = IC_MALLOC_ARRAY(int, 4);
    const int allocated = (values != NULL);
    free(values);

    // ic_bounded_loop.h
    int count = 0;
    IC_BOUNDED_WHILE(count < 3, 10)
    {
        count++;
    }

    // ic_co_job.h
    int steps = 0;
    const ic_co_job job = IC_MAKE_CO_JOB(ihc_cpp_co_step, ihc_cpp_co_step);
    ic_co_job_run(&job, &steps);
    ic_co_scheduler sched = ic_make_co_scheduler();
    const int scheduler_done = ic_co_scheduler_is_done(&sched);

    // numbers.h / ic_num_cast.h
    const i32 clamped = cast_i64_to_i32(INT64_MAX);

    // global_result.h / global_error.h / ic_typenum.h
    const VoidResult nothing = VoidResult_ok();
    const int valid_error = AppError_is_valid(AppError_Timeout);

    // ic_concurrency.h
    ic_atomic_i32 atom = ic_make_atomic(2);
    (void)ic_atomic_fetch_add(&atom, 1);

    return allocated + count + steps + scheduler_done + (clamped > 0) + nothing.ok + valid_error + ic_atomic_load(&atom);
}
