#include "Fission/core.hpp"

// --------------------------------------------------------------------------------
// Header-only Libraries

DISABLE_ALL_WARNINGS_BEGIN

#define FMT_HEADER_ONLY
#include "fmt/format.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"

DISABLE_ALL_WARNINGS_END

// --------------------------------------------------------------------------------
// Globals

namespace fission
{
	Engine engine;
}

// --------------------------------------------------------------------------------
// Source

using namespace fission;
#include "os.cpp"
#include "core.cpp"

// --------------------------------------------------------------------------------
// Platform

#if   defined(OS_WINDOWS)
#   include "platform_windows.cpp"
#elif defined(OS_MACOS)
#   include "platform_macos.cpp"
#elif defined(OS_LINUX)
#   include "platform_linux.cpp"
#elif defined(OS_ANDROID)
#   include "platform_android.cpp"
#endif

// --------------------------------------------------------------------------------
// Entry-points

auto OS_CALL Engine::render_main(void*) noexcept -> os::Thread_Result
{
    engine.setup();
    while (engine.render_frame());
    engine.shutdown();
    return {};
}

#ifdef os_main
os_main()
{
    if (engine.create(on_create()))
        return engine.exit_code;
    engine.run();
    engine.destroy();
	return engine.exit_code;
}
#endif
