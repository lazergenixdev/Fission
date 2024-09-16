#include <engine.hpp>
#include <pthread.h>
#include <android/native_window_jni.h>

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

    enum {
#define X(ID, NAME, SIG) ID,
        JAVA_FUNCTIONS
#undef X
    };

    Function functions [1] {
#define X(ID, NAME, SIG) { NAME, SIG },
            JAVA_FUNCTIONS
#undef X
    };
} java;

extern OS_MAIN();

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

extern "C"
JNIEXPORT jint JNICALL JAVA_NATIVE_FUNCTION(create)(JNIEnv* env, jobject thiz)
{
    LOGI("Setting up Java for C++...");

    env->GetJavaVM(&java.vm);
    java.activity       = env->NewGlobalRef(thiz);
    java.activity_class = env->GetObjectClass(thiz);
    LOGIf("Got class: %p", java.activity_class);

    for (auto&& fn: java.functions) {
        fn.method_id = env->GetMethodID(java.activity_class, fn.name, fn.signature);
        LOGIf("Got method id = %i", fn.method_id);
    }

    os_create_mutex(engine.render_lock);
    os_lock_mutex(engine.render_lock);
    pthread_create(&engine.main_thread, nullptr, android_main, nullptr);
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL JAVA_NATIVE_FUNCTION(destroy)(JNIEnv* env, jobject)
{
    LOGI("Destroying main thread...");
    pthread_join(engine.main_thread, nullptr);
    os_destroy_mutex(engine.render_lock);

    env->DeleteGlobalRef(java.activity);
    java.activity = nullptr;
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
JAVA_NATIVE_FUNCTION(createGraphics)(JNIEnv *env, jclass, jobject java_surface)
{
    engine.window = ANativeWindow_fromSurface(env, java_surface);
    gfx.create();
    pthread_mutex_unlock(&engine.render_lock);
}

extern "C"
JNIEXPORT void JNICALL
JAVA_NATIVE_FUNCTION(resizeGraphics)(JNIEnv *env, jclass, jobject java_surface, jint format, jint width, jint height)
{
    LOGIf("Surface (%p) resized: [format=%d,width=%d,height=%d]", java_surface, format, width, height);
    //update_swap_chain(0,0);
}

extern "C"
JNIEXPORT void JNICALL
JAVA_NATIVE_FUNCTION(destroyGraphics)(JNIEnv *env, jclass)
{
    engine.want_exit = true;
    LOGI("Destroying Graphics...");
    gfx.destroy();
}

namespace os {
    void show_dialog(char const* title, char const* message) {
        jstring java_title   = env->NewStringUTF(title);
        jstring java_message = env->NewStringUTF(message);
        env->CallVoidMethod(java.activity, java.functions[Java::SHOW_DIALOG].method_id, java_title, java_message);
        env->DeleteLocalRef(java_message);
        env->DeleteLocalRef(java_title);
    }
}
