#pragma once
/////////////////////////////////////////////////////
// Put this in seperate header "posix_system.hpp"
#include <pthread.h>

namespace os { using Mutex = pthread_mutex_t; }
#define os_mutex_create(p_mutex) pthread_mutex_init(p_mutex, nullptr)
#define os_mutex_destroy(mutex)  pthread_mutex_destroy(&(mutex))
#define os_mutex_lock(mutex)     pthread_mutex_lock(&(mutex))
#define os_mutex_unlock(mutex)   pthread_mutex_unlock(&(mutex))

namespace os {
    using Thread = pthread_t;
    using Thread_Result = void*;
}
#define os_thread_start(function, p_arg, p_thread) \
    pthread_create(p_thread, nullptr, function, p_arg)
#define os_thread_join(thread) \
    pthread_join(thread, nullptr)
/////////////////////////////////////////////////////

#define OS_CALL

#define FISSION_PLATFORM_VULKAN_EXTENSION_NAMES \
    "VK_KHR_wayland_surface"
//    "VK_KHR_xcb_surface", \
//    "VK_KHR_xlib_surface"

#define _os_main() \
int main(int argc, char* argv[])

#include <thread>
#include <vector>

struct GLFWwindow;

__FISSION_BEGIN__

namespace platform
{
    struct Instance {
    //    Instance(int argc, char* argv[]):
    //        argv(argv+0, argv+argc)
    //    {}
    //    std::vector<const char*> argv;
    };

    struct Window {
        GLFWwindow* _glfw_window;
    };

    struct Display {

    };
}

__FISSION_END__

