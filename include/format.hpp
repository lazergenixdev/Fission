
#if defined(FISSION_COMPILER_MSVC)
#	pragma warning (push, 2)
#elif defined(FISSION_COMPILER_GCC) || defined(FISSION_COMPILER_CLANG)
#   pragma GCC diagnostic push
    FISSION_DISABLE_WARNING("-Weverything")
#endif

#include <fmt/format.h>

#if defined(FISSION_COMPILER_MSVC)
#	pragma warning (pop)
#elif defined(FISSION_COMPILER_GCC) || defined(FFISSION_COMPILER_CLANG)
#   pragma GCC diagnostic pop
#endif