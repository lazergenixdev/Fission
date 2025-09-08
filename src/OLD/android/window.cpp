#include "../internal.hpp"
#include <Fission/core/engine.hpp>
#include <Fission/core/input/Keys.hpp>
#include <android/native_window_jni.h>
#include <android/input.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <pthread.h>
#include <fmt/format.h>

using fmt::format;
using namespace fs;

fs::string platform_version = "Android 19";
fs::string cpu_name = "[CPU NAME]";

#define __ANDROID_NAMESPACE__ com_example_testfission

#define JOIN(A,B,C,D) A ## B ## C ## D
#define JAVA_NATIVE_FUNCTION2(NAMESPACE, NAME) JOIN(Java_, NAMESPACE, _MainActivity_, NAME)
#define JAVA_NATIVE_FUNCTION(NAME) JAVA_NATIVE_FUNCTION2(__ANDROID_NAMESPACE__, NAME)

#define JAVA_FUNCTIONS \
X(SHOW_DIALOG, "showDialog", "(Ljava/lang/String;Ljava/lang/String;)V")

struct Java {
    struct Function {
        char const* name;
        char const* signature;
        jmethodID   method_id;
    };

    JavaVM* vm {};
    jclass  activity_class {};
    jobject activity {};
    AAssetManager* asset_manager {};

    enum {
#define X(ID, NAME, SIG) ID,
        JAVA_FUNCTIONS
#undef X
        JAVA_FUNCTION_COUNT,
    };

    Function functions [JAVA_FUNCTION_COUNT] {
#define X(ID, NAME, SIG) { NAME, SIG },
            JAVA_FUNCTIONS
#undef X
    };
} java;

#if 0
namespace os {
    thread_local JNIEnv* env;
    
    void initialize()
    {
        LOGI("Attaching...");
        java.vm->AttachCurrentThread(&os::env, nullptr);
    }

    void shutdown()
    {
        LOGI("Detaching...");
        java.vm->DetachCurrentThread();
    }
}
#endif

extern "C"
JNIEXPORT jint JNICALL JAVA_NATIVE_FUNCTION(create)(JNIEnv* env, jobject thiz)
{
    log::debug(PLATFORM_"Setting up Java for C++...");

    env->GetJavaVM(&java.vm);
    java.activity       = env->NewGlobalRef(thiz);
    java.activity_class = env->GetObjectClass(thiz);
    log::verbose(format(PLATFORM_"Got class: {}", (void*)java.activity_class));

    for (auto&& fn: java.functions) {
        fn.method_id = env->GetMethodID(java.activity_class, fn.name, fn.signature);
        log::verbose(format(PLATFORM_" - \"{}\" id = {}", fn.name, (size_t)fn.method_id));
    }

    return 0;
}

extern "C"
JNIEXPORT jint JNICALL JAVA_NATIVE_FUNCTION(destroy)(JNIEnv* env, jobject)
{
    env->DeleteGlobalRef(java.activity);
    java.activity = nullptr;
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
JAVA_NATIVE_FUNCTION(createGraphics)(JNIEnv *env, jclass, jobject java_surface)
{
    engine.window._native = ANativeWindow_fromSurface(env, java_surface);
    if (engine.create(on_create())) return;
}

extern "C"
JNIEXPORT void JNICALL
JAVA_NATIVE_FUNCTION(test)(JNIEnv *env, jclass, jobject java_asset_manager) {
    log::verbose("Julie says hi!");

    AAssetManager* asset_manager = AAssetManager_fromJava(env, java_asset_manager);
    AAssetDir* dir = AAssetManager_openDir(asset_manager, "autumn_field_puresky_8k");

    log::debug(format("dir = {}", (void*)dir));

    const char* filename = NULL;

    do {
        filename = AAssetDir_getNextFileName(dir);
        log::debug(format("file = {}", (filename ? filename : "[NULL]")));
    } while (filename != NULL);

    AAssetDir_close(dir);

    java.asset_manager = asset_manager;
}

extern "C"
JNIEXPORT void JNICALL
JAVA_NATIVE_FUNCTION(destroyGraphics)(JNIEnv *env, jclass)
{
    engine.destroy();
}

extern "C"
JNIEXPORT void JNICALL
JAVA_NATIVE_FUNCTION(addTouchEvent)(JNIEnv *env, jclass, jint action, jfloat x, jfloat y)
{
    engine.window.mouse_position = {(int)roundf(x), (int)roundf(y)};
    switch (action) {
        default:
        break; case AMOTION_EVENT_ACTION_DOWN: {
            engine.window.event_queue.append({
                .type = fs::EventType::Event_Key_Down,
                .key_down = {
                    .key_id = fs::keys::Mouse_Left,
                }
            });
        }
        break; case AMOTION_EVENT_ACTION_UP: {
            engine.window.event_queue.append({
                 .type = fs::EventType::Event_Key_Up,
                 .key_down = {
                     .key_id = fs::keys::Mouse_Left,
                 }
            });
        }
    }
}

namespace os {
    void show_dialog(char const* title, char const* message) {
#if 0
        jstring java_title   = env->NewStringUTF(title);
        jstring java_message = env->NewStringUTF(message);
        env->CallVoidMethod(java.activity, java.functions[Java::SHOW_DIALOG].method_id, java_title, java_message);
        env->DeleteLocalRef(java_message);
        env->DeleteLocalRef(java_title);
#endif
    }

    void show_error_dialog(fs::string const& title, fs::string const& message)
    {
        (void)title;
        (void)message;
    }

    void log(int level, fs::string const& message) {
        static_assert(fs::log::Verbose + 2 == ANDROID_LOG_VERBOSE);
        __android_log_write(level + 2, "Fission", message.str().c_str());
    }
}

bool Window::create(Window_Create_Info const&) {
    return false;
}

Window::~Window() = default;

void Window::show() {

}

void Window::close() {

}

void Window::toggle_using_mouse_deltas() {

}

inline struct timespec temp;
#define nb 1'000'000'000

auto fs::timestamp() -> s64 {
    clock_gettime(CLOCK_MONOTONIC, &temp);
    return temp.tv_sec * nb + temp.tv_nsec; // <- this is fucking garbage
}

auto fs::seconds_elasped_and_reset(s64& last) -> f64 {
    auto current = fs::timestamp();
    auto duration = double(current - last) / 1e9;
    last = current;
    return duration;
}

void* load_entire_file(const char* filename, int* size) {
    AAsset* asset = AAssetManager_open(java.asset_manager, filename, AASSET_MODE_BUFFER);
    log::info(format("Opened asset={} file=\"{}\"", (void*)asset, filename));
    int length = AAsset_getLength(asset);
    void* data = malloc(length);
    AAsset_read(asset, data, length);
    *size = length;
    AAsset_close(asset);
    return data;
}
