#include <pthread.h>

// OS Mutex and Thread implementations

namespace os { using Mutex = pthread_mutex_t; }
#define os_mutex_create(p_mutex) pthread_mutex_init(p_mutex, nullptr)
#define os_mutex_destroy(mutex)  pthread_mutex_destroy(&(mutex))
#define os_mutex_lock(mutex)     pthread_mutex_lock(&(mutex))
#define os_mutex_unlock(mutex)   pthread_mutex_unlock(&(mutex))

namespace os { using Thread = pthread_t; }
namespace os { using Thread_Result = void*; }
#define os_thread_start(function, p_arg, p_thread) pthread_create(p_thread, nullptr, function, p_arg)
#define os_thread_join(thread)                     pthread_join(thread, nullptr)
