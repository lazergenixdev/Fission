#include "Fission/core.hpp"
#include <iostream>

void log::log(int level, string const& message)
{
    static constexpr char const * level_strings [] {
        "VERBOSE", "DEBUG", "INFO", "WARN", "ERROR",
    };

    SYSTEMTIME lt;
    GetLocalTime(&lt);

    auto buffer = fmt::format("{:02}:{:02}:{:02}.{:03}   {:>9}   {}\n",
        lt.wHour, lt.wDay, lt.wSecond, lt.wMilliseconds,
        level_strings[level], message.as<std::string_view>()
    );

//    engine.logger.file << buffer;
	printf("%s\n", buffer.c_str());
#if defined(OS_WINDOWS)
    OutputDebugStringA(buffer.c_str());
   // Console_Window::get().write(level, buffer);
#endif
}

auto Engine::create(Defaults const& defaults) -> bool
{
	if (os::init())
		return true;
    log::info("Creating Fission Engine...");
	NOT_USED(defaults);
	Sleep(1000);
	return false;
}

void Engine::run()
{
}

void Engine::destroy()
{
}

auto Engine::setup() -> bool
{
	return false;
}

auto Engine::render_frame() -> bool
{
	return false;
}

void Engine::shutdown()
{
}
