#include "Fission/core.hpp"

// --------------------------------------------------------------------------------
// Globals

namespace fission
{
    u8 _scratch_memory[1_MiB];

	Engine engine;
    Arena logging_arena;
    Arena scratch_arena {.start = _scratch_memory, .capacity = sizeof(_scratch_memory)};
    Mutex logging_mutex;
    const char* logging_prefix;
    int minimum_log_level {log::Verbose};
	FILE* logging_file;
}

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
// Source

#include "base.cpp"
#include "core.cpp"
#include "graphics.cpp"

// --------------------------------------------------------------------------------
// Entry-points

using namespace fission;

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
