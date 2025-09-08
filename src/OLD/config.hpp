#include "Fission/core/engine.hpp"
#include "fmt/format.h"

#define PLATFORM_LOG_PREFIX "(" FS_PLATFORM_NAME ") "

FISSION_NAMESPACE_BEGIN

#define MAJOR(X)       static constexpr u32  version_major      = X;
#define MINOR(X)       static constexpr u32  version_minor      = X;
#define PATCH(X)       static constexpr u32  version_patch      = X;
#define COPYRIGHT(X)   static constexpr auto copyright_year     = X;
#define COMPANY(X)     static constexpr auto company_name       = X;
#define DESCRIPTION(X) static constexpr auto engine_description = X;
#include "version.inl"
#undef MAJOR
#undef MINOR
#undef PATCH
#undef COPYRIGHT
#undef COMPANY
#undef DESCRIPTION

struct Window_Create_Info {
    string      title;
    int         width;
    int         height;
    Window_Mode mode;
    int         display_index;
};

struct Graphics_Create_Info {
	Window*          window;
	VkPresentModeKHR present_mode;
	bool             debug;
};

FISSION_NAMESPACE_END

// Thanks C++, for not giving us the most basic of things
#define for_n(N) \
    for (decltype(N) i = 0; i < (N); ++i)

#ifdef assert
#undef assert
#endif
