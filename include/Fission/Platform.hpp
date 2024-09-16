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

// DOCUMENTATION
//
// ----------------------- Mutexes ------------------------
// 
// Type: `os::Mutex`
// 
// Functions (macros):
//  - os_mutex_create(p_mutex)
//  - os_mutex_destroy(mutex)
//  - os_mutex_lock(mutex)
//  - os_mutex_unlock(mutex)
//
// ----------------------- Threads ------------------------
// 
// Type: `os::Thread`
// 
// Thread Function: (*void) -> os::Thread_Result
// note:
//      When returning from a thread function,
//      just do `return {}`.
// 
// Functions (macros):
//  - os_thread_start(function, p_arg, p_thread)
//  - os_thread_join(thread)
//
// ----------------------- Logging ------------------------
// 
// Functions:
//  - os::log(level, message)
// 
// see "core/log.hpp" for more detail
//
// -------------------- Dialog Boxes ----------------------
// 
// Functions:
//  - os::show_error_dialog(title, message)
//  - os::show_file_dialog(...) NOT IMPLEMENTED
//

#pragma once
#include <Fission/config.hpp>

#if   defined(FISSION_PLATFORM_HEADLESS)
#   include "platform/headless.hpp"
#elif defined(FISSION_PLATFORM_WINDOWS)
#   include "platform/windows.hpp"
#elif defined(FISSION_PLATFORM_LINUX)
#   include "platform/linux.hpp"
#elif defined(FISSION_PLATFORM_ANDROID)
#   include "platform/android.hpp"
#elif defined(FISSION_PLATFORM_MACOS)
#   include "platform/macos.hpp"
#elif defined(FISSION_PLATFORM_IOS)
#   include "platform/ios.hpp"
#endif

/**
 *	MIT License
 *
 *	Copyright (c) 2021-2025 lazergenixdev
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
