// Compiled as its own translation unit with ic_memory.h as the very first include, followed by
// ic_bounded_loop.h (which ic_memory.h does not include), so the build fails if the macros of either
// need anything their header does not include itself.
#include "ironclib/ic_memory.h"
#include "ironclib/ic_bounded_loop.h"

typedef struct StandaloneAligned
{
    IC_ALIGNAS_TYPE(double) ic_byte data[sizeof(double)];
} StandaloneAligned;

int standalone_memory_and_loops_value(void);
int standalone_memory_and_loops_value(void)
{
    int* const values = IC_MALLOC_ARRAY(int, 4);
    const int allocated = (values != NULL);
    free(values);

    int count = 0;
    IC_BOUNDED_WHILE(count < 100, 10)
    {
        count++;
    }
    IC_BOUNDED_DO_WHILE(count < 0, 5)
    {
        count++;
    }

    return allocated + count + (int)IC_ALIGNOF(StandaloneAligned);
}
