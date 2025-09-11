#include "Fission/core.hpp"
#include <cstdlib>

BEGIN_NAMESPACE(fission);

auto Arena::create(size_t max_size) -> Result
{
    //! TODO: own alloc
    start = malloc(max_size);
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

auto Dynamic_Arena::alloc(size_t size) -> void*
{
    if (allocated + size > capacity)
    {
        size_t new_capacity = 3 * (allocated + size) / 2;
        start = realloc(start, new_capacity);
        capacity = new_capacity;
    }
    void* ptr = (byte*)start + allocated;
    allocated += size;
    return ptr;
}

END_NAMESPACE();
