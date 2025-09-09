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
    void* ptr = (byte*)start + allocated;
    allocated += size;
    return ptr;
}

END_NAMESPACE();

#ifdef TEST_FMT
auto fmt::formatter<fission::string>::format(fission::string s, format_context& ctx) const -> format_context::iterator
{
    return formatter<string_view>::format(s.as<std::string_view>(), ctx);
}
#endif
