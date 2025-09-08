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
#include "base.hpp"

// --------------------------------------------------------------------------------
// Detect OS

#if defined(_WIN32)
#	define OS_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
#	include <TargetConditionals.h>
	/* TARGET_OS_MAC exists on all the platforms
	* so we must check all of them (in this order)
	* to ensure that we're running on MAC
	* and not some other Apple platform */
#	if TARGET_IPHONE_SIMULATOR == 1
#		define OS_IOS
#	elif TARGET_OS_IPHONE == 1
#		define OS_IOS
#	elif TARGET_OS_MAC == 1
#		define OS_MACOS
#	else
#		pragma message("[Fission] Unknown Apple platform!")
#	endif
/* We also have to check __ANDROID__ before __linux__
 * since android is based on the linux kernel
 * it has __linux__ defined */
#elif defined(__ANDROID__)
#	define OS_ANDROID
#elif defined(__linux__)
#	define OS_LINUX
#else
#	pragma message("[Fission] Unknown platform!")
#endif

// --------------------------------------------------------------------------------
// OS includes

#if defined(OS_WINDOWS)
#   include <Windows.h>
#elif defined(OS_LINUX) || defined(OS_ANDROID) || defined(OS_MACOS)
#   include <pthread.h>
#   include "GLFW/glfw3.h"
#elif defined(OS_ANDROID)
#   include <pthread.h>
#   include <android/native_activity.h>
#endif

// --------------------------------------------------------------------------------
// OS entry point / calling convention

#if defined(OS_WINDOWS)
#   define OS_CALL WINAPI
#   define os_main() int APIENTRY ::WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
#elif defined(OS_LINUX) || defined(OS_MACOS)
#   define OS_CALL
#   define os_main() int main(int argc, char* argv[])
#elif defined(OS_ANDROID)
#   define OS_CALL
#endif

// --------------------------------------------------------------------------------
// Vulkan setup

#if defined(OS_WINDOWS)
#   define VK_USE_PLATFORM_WIN32_KHR 1
#   define OS_VULKAN_EXTENSION_NAMES "VK_EXT_win32_surface"
#elif defined(OS_LINUX)
#   define OS_VULKAN_EXTENSION_NAMES "VK_KHR_wayland_surface", "VK_KHR_xcb_surface", "VK_KHR_xlib_surface"
#elif defined(OS_MACOS)
#define FISSION_PLATFORM_VULKAN_EXTENSION_NAMES "VK_EXT_metal_surface"
#elif defined(OS_ANDROID)
#   define VK_USE_PLATFORM_ANDROID_KHR 1
#   define OS_VULKAN_EXTENSION_NAMES "VK_EXT_android_surface"
#endif

// --------------------------------------------------------------------------------

BEGIN_NAMESPACE(os)
    
// --------------------------------------------------------------------------------
// Type: `Mutex`
// 
// Functions (macros): (all return true on failure)
//  - os_mutex_create  (*mutex) -> bool
//  - os_mutex_destroy (mutex)
//  - os_mutex_lock    (mutex) -> bool
//  - os_mutex_unlock  (mutex) -> bool
//
#if defined(OS_WINDOWS)
    using Mutex = HANDLE;
#   define os_mutex_create(p_mutex) ((*(p_mutex) = CreateMutexW(nullptr, FALSE, nullptr)) == NULL)
#   define os_mutex_destroy(mutex)  (CloseHandle(mutex) == 0)
#   define os_mutex_lock(mutex)     (WaitForSingleObject(mutex, INFINITE) == WAIT_FAILED)
#   define os_mutex_unlock(mutex)   (ReleaseMutex(mutex) == 0)
#elif defined(OS_LINUX) || defined(OS_ANDROID) || defined(OS_MACOS)
    using Mutex = pthread_mutex_t;
#   define os_mutex_create(p_mutex) pthread_mutex_init(p_mutex, nullptr)
#   define os_mutex_destroy(mutex)  pthread_mutex_destroy(&(mutex))
#   define os_mutex_lock(mutex)     pthread_mutex_lock(&(mutex))
#   define os_mutex_unlock(mutex)   pthread_mutex_unlock(&(mutex))
#endif

// --------------------------------------------------------------------------------
// Type: `Thread`, `Thread_Result`
// 
// Thread Function: (*void) -> Thread_Result
//   Note: When returning from a thread function, just do `return {}`.
// 
// Functions (macros): (all return true on failure)
//  - os_thread_start(function, p_arg, p_thread)
//  - os_thread_join(thread)
//
#if defined(OS_WINDOWS)
    using Thread = HANDLE;
    using Thread_Result = DWORD;
#   define os_thread_start(function, p_arg, p_thread) ((*(p_thread) = CreateThread(nullptr, 0, function, p_arg, 0, nullptr)) == NULL)
#   define os_thread_join(thread)                     (WaitForSingleObject(thread, INFINITE), CloseHandle(thread))
#elif defined(OS_LINUX) || defined(OS_ANDROID) || defined(OS_MACOS)
    using Thread        = pthread_t;
    using Thread_Result = void*;
#   define os_thread_start(function, p_arg, p_thread) pthread_create(p_thread, nullptr, function, p_arg)
#   define os_thread_join(thread)                     pthread_join(thread, nullptr)
#endif

// --------------------------------------------------------------------------------
// OS Display

#if defined(OS_WINDOWS)
struct Display
{
	HMONITOR _handle {};
};
#elif defined(OS_LINUX) || defined(OS_MACOS)
struct Display
{
};
#elif defined(OS_ANDROID)
struct Display
{
};
#endif

// --------------------------------------------------------------------------------
// OS Window

#if defined(OS_WINDOWS)
struct Window
{
    HWND _handle {};
    int  _mouse_wheel_delta {};
    u32  _flags {};

protected:
    static LRESULT CALLBACK _setup_callback(HWND, UINT, WPARAM, LPARAM) noexcept;
};
#elif defined(OS_LINUX) || defined(OS_MACOS)
struct Window
{
    GLFWwindow* _window;
};
#elif defined(OS_ANDROID)
struct Window
{
    struct ANativeWindow* _native;
};
#endif

// --------------------------------------------------------------------------------
// General Functions

// --------------------------------------------------------------------------------

END_NAMESPACE()

/**
 *	MIT License
 *
 *	Copyright (c) 2025 lazergenixdev
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */
