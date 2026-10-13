// Compiled as its own translation unit with ic_co_job.h as the very first include,
// so the build fails if its macros need anything the header does not include itself.
#include "ironclib/ic_co_job.h"

static void standalone_co_job_step(void* data)
{
    int* const count = (int*)data;
    (*count)++;
}

int standalone_co_job_run(void);
int standalone_co_job_run(void)
{
    int count = 0;
    const ic_co_job job = IC_MAKE_CO_JOB(standalone_co_job_step, standalone_co_job_step);
    ic_co_job_run(&job, &count);
    return count;
}
