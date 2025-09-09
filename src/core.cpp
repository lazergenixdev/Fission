#include "Fission/core.hpp"
#ifdef TEST_FMT
#include "fmt/chrono.h"
#endif
#include <unistd.h>

namespace os { auto init() -> fission::Result; }

BEGIN_NAMESPACE(fission)

Window::~Window() {
	//! NOTE: Exiting application, no need to do anything
	log::info("YOU ARE TERMINATED");
}

Logging_Timestamp Logging_Timestamp::now()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    std::time_t t = system_clock::to_time_t(now);
    std::tm local {};
#if defined(OS_WINDOWS)
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    return {
        .date = ((u32(local.tm_year) + 1900) << 16)
              | ((u32(local.tm_mon) + 1) << 8)
              | (u32(local.tm_mday)),
        .time = (u32(local.tm_hour) << 16)
              | (u32(local.tm_min) << 8)
              | (u32(local.tm_sec)),
        .milliseconds = u32(ms),
    };
}

void log::write_log_from_logging_arena(int level)
{
    static constexpr const char * level_colors [] {
        "\x1b[90m", "\x1b[96m", "\x1b[0m", "\x1b[93m", "\x1b[91m",
    };
    // TODO: Log to log file
    fputs(level_colors[level], stdout);
    fwrite(logging_arena.start, 1, logging_arena.allocated, stdout);
    if (level != Info) fputs("\x1b[0m", stdout);
#if defined(OS_WINDOWS) // Also output to debugger (if available)
    OutputDebugStringA((char*)logging_arena.start);
#endif
}

auto Engine::create(Defaults const& defaults) -> Result
{
	if (os::init()) return Failed;
    logging_arena.create(1_KiB);
    os_mutex_create(&logging_mutex);
    log::info("Creating Fission Engine...");
    /*
	// setup the console early so we can use it as soon as possible
	console_layer.setup_console_api();
	add_engine_console_commands();
    */
    Window::Create_Info window_info = {
        .width = defaults.window_width,
        .height = defaults.window_height,
    };
    if (window.create(window_info)) return Failed;
    
    //Graphics::Create_Info graphics_info = {
	//	.window = &window,
    //    .debug = true,
    //};
	//if (graphics.create(graphics_info)) return Failed;
    
    //window.show();

    /*
	if (create_screenshot_buffer()) return true;
    if (create_layers())            return true;

	engine.current_scene = on_create_scene({});
    engine.flags |= Engine::Running;

    log::debug("Creating render thread...");
	if (os_thread_start(fs::render_main, nullptr, &engine.render_thread)) {
		log::error("Failed to start render thread!");
		return true;
	}
    */
	return Success;
}

void Engine::destroy()
{
#ifdef TEST_FMT
    log::verbose(fmt::format("{}", __PRETTY_FUNCTION__));
#else
    log::verbose(__PRETTY_FUNCTION__);
#endif
}

auto Engine::setup() -> Result
{
	return Success;
}

auto Engine::render_frame() -> Result
{
	return Success;
}

void Engine::shutdown()
{
}

END_NAMESPACE()
