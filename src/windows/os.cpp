#include <Fission/core/engine.hpp>
#include <Fission/core/log.hpp>

using namespace fs;

void os::log(int level, string const& message)
{
    static constexpr char const * level_strings [] {
        "VERBOSE", "DEBUG", "INFO", "WARN", "ERROR",
    };

    SYSTEMTIME lt;
    GetLocalTime(&lt);

    auto buffer = std::format("{:02}:{:02}:{:02}.{:03}   {:>9}   {}\n",
        lt.wHour, lt.wDay, lt.wSecond, lt.wMilliseconds,
        level_strings[level], message.view()
    );

    OutputDebugStringA(buffer.c_str());
    engine.logger.file << buffer;
}

void os::show_error_dialog(fs::string const& title, fs::string const& message)
{
    MessageBoxA(0, message.str().c_str(), title.str().c_str(), MB_ICONERROR | MB_OK);
    //SendNotifyMessageW(engine.window, WM_USER_SHOW_DIALOG, (WPARAM)title, (LPARAM)message);
}

void Engine::run() {
    log::verbose("(Win32) starting message loop...");

    BOOL bRet {};
    MSG msg;
    while ((bRet = GetMessageW(&msg, nullptr, 0, 0)) != 0) {
        if (bRet == -1) {
        	log::error("An Error Occured!"
                       "(but idk what kind of error, "
                       "this never happens to me, I swear)");
            engine.exit_code = EXIT_FAILURE;
            break;
        }
        else {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    // invalidate main window
    engine.window._handle = NULL;
}

struct PerformanceFrequency {
	PerformanceFrequency() {
		QueryPerformanceFrequency(&raw);
		value = (double)raw.QuadPart;
	}
	double value;
	LARGE_INTEGER raw;
} _freq;

s64 fs::timestamp() {
	LARGE_INTEGER o;
	QueryPerformanceCounter(&o);
	return o.QuadPart;
}

double fs::seconds_elasped(s64 start, s64 end) {
	return double(end - start) / _freq.value;
}

double fs::seconds_elasped_and_reset(s64& last) {
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	double elapsed = double(now.QuadPart - last) / _freq.value;
	last = now.QuadPart;
	return elapsed;
}
