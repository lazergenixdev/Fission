#include <Fission/core/engine.hpp>
#include <inttypes.h>

#if defined(FISSION_PLATFORM_WINDOWS)
#define PLATFORM_ "(Win32) "
#elif defined(FISSION_PLATFORM_ANDROID)
#define PLATFORM_ "(Android) "
#endif

__FISSION_BEGIN__

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
    int width, height;
    Window_Mode mode;
    int display_index;
    string title;
};

struct Graphics_Create_Info {
	Window*          window;
	VkPresentModeKHR present_mode;
	bool             debug;
};

__FISSION_END__

#define for_n(N) \
    for (decltype(N) i = 0; i < (N); ++i)

#ifdef assert
#undef assert
#endif
