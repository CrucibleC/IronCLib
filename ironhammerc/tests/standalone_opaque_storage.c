// Compiled as its own translation unit with ic_opaque_storage.h as the very first include,
// so the build fails if its macros need anything the header does not include itself.
#include "ironclib/ic_opaque_storage.h"

#define STANDALONE_OPAQUE_SIZE  (sizeof(int) * 2)
#define STANDALONE_OPAQUE_ALIGN (IC_ALIGNOF(int))

IC_OPAQUE_STORAGE(StandaloneOpaque, STANDALONE_OPAQUE_ALIGN, STANDALONE_OPAQUE_SIZE)

typedef struct StandaloneOpaqueImpl
{
    int a;
    int b;
} StandaloneOpaqueImpl;

IC_OPAQUE_IMPL_ASSERT(StandaloneOpaqueImpl, STANDALONE_OPAQUE_ALIGN, STANDALONE_OPAQUE_SIZE)

size_t standalone_opaque_storage_size(void);
size_t standalone_opaque_storage_size(void)
{
    return sizeof(StandaloneOpaque);
}
