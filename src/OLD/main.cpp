#include "Fission/config.hpp"

FISSION_DISABLE_ALL_WARNINGS_BEGIN

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"

FISSION_DISABLE_ALL_WARNINGS_END

#include "config.hpp"
#include "engine.cpp"
#include "font.cpp"
#include "graphics.cpp"
#include "renderer_2d.cpp"
#include "console_layer.cpp"
#include "debug_layer.cpp"

#if   defined(FISSION_PLATFORM_WINDOWS)
#   include "platform_windows.cpp"
#elif defined(FISSION_PLATFORM_MACOS)
#   include "platform_macos.cpp"
#elif defined(FISSION_PLATFORM_LINUX)
#   include "platform_linux.cpp"
#elif defined(FISSION_PLATFORM_ANDROID)
#   include "platform_android.cpp"
#endif

FISSION_NAMESPACE_BEGIN

auto OS_CALL render_main(void*) noexcept -> os::Thread_Result
{
    engine.setup();

    while (engine.render_frame());

    engine.shutdown();
    return {};
}

FISSION_NAMESPACE_END

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
