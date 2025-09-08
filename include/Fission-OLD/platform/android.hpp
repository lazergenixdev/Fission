/**
 *	______________              _____
 *	___  ____/__(_)________________(_)____________
 *	__  /_   __  /__  ___/_  ___/_  /_  __ \_  __ \
 *	_  __/   _  / _(__  )_(__  )_  / / /_/ /  / / /
 *	/_/      /_/  /____/ /____/ /_/  \____//_/ /_/
 *
 *
 * @Author:       lazergenixdev@gmail.com
 * @Development:  (https://github.com/lazergenixdev/Fission)
 * @License:      MIT (see end of file)
 */
#pragma once
#include <Fission/config.hpp>
#include <android/log.h>
#include <android/native_activity.h>
#include <pthread.h>

#define VK_USE_PLATFORM_ANDROID_KHR 1
#define FISSION_PLATFORM_VULKAN_EXTENSION_NAMES \
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME

namespace fs { struct string; }

#define OS_CALL

namespace os
{
// ----------------------- Mutexes ------------------------

using Mutex = pthread_mutex_t;
#define os_mutex_create(p_mutex) pthread_mutex_init(p_mutex, nullptr)
#define os_mutex_destroy(mutex)  pthread_mutex_destroy(&(mutex))
#define os_mutex_lock(mutex)     pthread_mutex_lock(&(mutex))
#define os_mutex_unlock(mutex)   pthread_mutex_unlock(&(mutex))


// ----------------------- Threads ------------------------

using Thread = pthread_t;
using Thread_Result = void*;
#define os_thread_start(function, p_arg, p_thread) pthread_create(p_thread, nullptr, function, p_arg)
#define os_thread_join(thread)                     pthread_join(thread, nullptr)

}

__FISSION_BEGIN__

namespace platform
{
	struct Window
    {
        struct ANativeWindow* _native;
	};

	struct Display
    {
        int id;
    };
}

__FISSION_END__

//#define LOGIf(FMT, ...) __log(ANDROID_LOG_INFO, "Engine", "[%s] " FMT, __FUNCTION__, __VA_ARGS__)
//#define LOGI(MSG)       __log(ANDROID_LOG_INFO, "Engine", "[%s] " MSG, __FUNCTION__)
