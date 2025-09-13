#include "Fission/core.hpp"

// --------------------------------------------------------------------------------
// Globals

namespace fission
{
    byte _scratch_memory[1_MiB];
    byte _logger_memory[4_KiB];

    Arena scratch_arena {.start = _scratch_memory, .capacity = sizeof(_scratch_memory)};
	Logger logger {
		.arena = {.start = _logger_memory, .capacity = sizeof(_logger_memory)},
		.minimum_level = log::Info
	};
	Engine engine;
}
namespace os
{
	os::Info _info;
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
// Android platform layer built into application
// to allow for namespace to be changed per application
#endif

// --------------------------------------------------------------------------------
// Source

#include "base.cpp"
#include "core.cpp"
#include "graphics.cpp"

// --------------------------------------------------------------------------------
// Entry-points

using namespace fission;

auto OS_CALL Engine::render_main(void*) -> os::Thread_Result
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
