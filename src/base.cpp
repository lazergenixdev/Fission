#include "Fission/base.hpp"
#include <cstdlib>

BEGIN_NAMESPACE(fission);

auto Arena::create(size_t max_size) -> Result
{
    start = malloc(max_size); // TODO: own alloc
    capacity = max_size;
    return Success;
}
void Arena::destroy()
{
    if (start) free(start);
    *this = {};
}
auto Arena::alloc(size_t size) -> void*
{
    ASSERT(allocated + size <= capacity);
    void* ptr = (byte*)start + allocated;
    allocated += size;
    return ptr;
}

END_NAMESPACE();
