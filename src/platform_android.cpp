#include <jni.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <unistd.h>
#include "Fission/core.hpp"

#define JOIN(A,B,C,D) A ## B ## C ## D
#define JAVA_NATIVE_FUNCTION_HELPER(NAMESPACE, NAME) JOIN(Java_, NAMESPACE, _MainActivity_, NAME)
#define JAVA_NATIVE_FUNCTION(NAME) JNICALL JAVA_NATIVE_FUNCTION_HELPER(__ANDROID_NAMESPACE__, NAME)

BEGIN_NAMESPACE(os)

auto init() -> fission::Result
{
	return fission::Success;
}

int fatal_error(string error, string message, source_location location)
{
    NOT_USED(error, message, location);
    return 0;
}

END_NAMESPACE()

BEGIN_NAMESPACE(fission)

void log::write_log_from_logger(int level)
{
    __android_log_write(level + ANDROID_LOG_VERBOSE, "Fission Engine", (char*)logger.arena.start);
}

auto ticks() -> u64
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return u64(1e9) * u64(ts.tv_sec) + u64(ts.tv_nsec);
}

auto seconds_elapsed_and_reset(u64& t) -> f64
{
    auto now = ticks();
    auto d = now - t;
    t = now;
    return f64(d) / u64(1e9);
}

void Engine::run()
{}

auto Window::create(Create_Info const&) -> Result
{
    return Success;
}

extern "C"
{
    JNIEXPORT void JAVA_NATIVE_FUNCTION(createGraphics)(JNIEnv *env, jclass, jobject java_surface)
    {
        engine.window._native = ANativeWindow_fromSurface(env, java_surface);
        if (engine.create(on_create())) return;
    }

    JNIEXPORT void JAVA_NATIVE_FUNCTION(addTouchEvent)(JNIEnv*, jclass, jint action, jfloat x, jfloat y)
    {
        const int ACTION_UP = 1;
        engine.window.mouse_position = {(int)roundf(x), (int)roundf(y)};
        auto& event = engine.window.event_queue[engine.window.event_tail];
        event.type = (action == ACTION_UP? Event_Key_Up : Event_Key_Down);
        event.key_down.key_id = u32(32);
        engine.window.event_tail = (engine.window.event_tail + 1) % array_count(engine.window.event_queue);
    }
}

END_NAMESPACE()
