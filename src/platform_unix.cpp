#include "Fission/core/engine.hpp"

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

inline struct timespec temp;
#define nb 1'000'000'000

auto fs::timestamp() -> s64 {
    clock_gettime(CLOCK_MONOTONIC, &temp);
    return temp.tv_sec * nb + temp.tv_nsec; // <- this is fucking garbage
}

auto fs::seconds_elasped_and_reset(s64& last) -> f64 {
    auto current = timestamp();
    auto duration = double(current - last) / 1e9;
    last = current;
    return duration;
}