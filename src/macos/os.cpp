#include <Fission/core/engine.hpp>
#include <ctime>
#include <format.hpp>
#include <GLFW/glfw3.h>
#include "../internal.hpp"
#include <time.h>
#include <sys/sysctl.h> // --> sysctl

char _platform_version_string_buffer[64];

fs::string platform_version = []() {
    
    // https://stackoverflow.com/a/65649178
    char    os_temp [20] = "";
    char   *os_temp_ptr  = os_temp;
    size_t  os_temp_len  = sizeof(os_temp);
    size_t  os_temp_left = 0;
    int     rslt         = 0;

    int major = 0;
    int minor = 0;
    int point = 0;
    
    rslt = sysctlbyname ( "kern.osproductversion", os_temp, &os_temp_len, NULL, 0 );
    if (rslt != 0) {
        // just silently fail, who cares
        goto format;
    }
    
    os_temp_left = os_temp_len; /* length of string returned */
    {
        int temp = atoi ( os_temp_ptr );
        major = temp;
    }
    major = atoi ( os_temp_ptr );
    
    while ( os_temp_left > 0 && *os_temp_ptr != '.' )
    {
        os_temp_left--;
        os_temp_ptr++;
    }
    os_temp_left--;
    os_temp_ptr++;
    minor = atoi ( os_temp_ptr );
    
    while ( os_temp_left > 0 && *os_temp_ptr != '.' )
    {
        os_temp_left--;
        os_temp_ptr++;
    }
    os_temp_left--;
    os_temp_ptr++;
    point = atoi ( os_temp_ptr );
    
    format:
    int len = snprintf(_platform_version_string_buffer,
        sizeof(_platform_version_string_buffer),
        "Mac OS X %d.%d.%d",
        major, minor, point);
    
    return fs::string { _platform_version_string_buffer, len };
}();

using namespace fs;

void os::log(int level, string const& message) {
    static constexpr char const * level_strings [] {
        "VERBOSE", "DEBUG", "INFO", "WARN", "ERROR"
    };

    static constexpr char const * level_colors [] {
        "\033[90m", "\033[96m", "\033[0m", "\033[93m", "\033[91m"
    };

    time_t rawtime;
    time(&rawtime);

    struct tm* lt = localtime(&rawtime);

    auto buffer = fmt::format("{}{:02}:{:02}:{:02}.{:03}   {:>9}   {}{}\n",
            level_colors[level],
            lt->tm_hour, lt->tm_min, lt->tm_sec, 0,
            level_strings[level], message.view(), "\033[0m"
            );

    printf("%s", buffer.c_str());
}

void os::show_error_dialog(string const&, string const&) {
    // not implemented
}

__FISSION_BEGIN__
    
inline struct timespec temp;
#define nb 1'000'000'000

auto timestamp() -> s64 {
    clock_gettime(CLOCK_MONOTONIC, &temp);
    return temp.tv_sec * nb + temp.tv_nsec; // <- this is fucking garbage
}

auto seconds_elasped_and_reset(s64& last) -> f64 {
    auto current = timestamp();
    auto duration = double(current - last) / 1e9;
    last = current;
    return duration;
}

void Engine::run() {
    log::verbose(PLATFORM_"starting message loop...");

    log::verbose(fmt::format("window = {}", (void*)engine.window._glfw_window));

    while (!glfwWindowShouldClose(engine.window._glfw_window))
        glfwWaitEvents();

    // invalidate main window
    //engine.window._handle = NULL;
}

__FISSION_END__

