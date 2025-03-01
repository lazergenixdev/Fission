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
//  - os_mutex_lock(mutex) -> bool
//       returns true on failure
//  - os_mutex_unlock(mutex) -> bool
//       returns true on failure
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
//       returns true on failure
//  - os_thread_join(thread)
//       returns true on failure
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
#include <fstream>

namespace fs { struct string; }

namespace os
{
    void log(int level, fs::string const& message);
    void show_error_dialog(fs::string const& title, fs::string const& message);
}

FISSION_NAMESPACE_BEGIN

namespace log
{
    enum {
        Verbose = 0,
        Debug   = 1,
        Info    = 2,
        Warn    = 3,
        Error   = 4,
        LEVEL_COUNT
    };

    inline void verbose (string const& message) { os::log(Verbose, message); }
    inline void debug   (string const& message) { os::log(Debug  , message); }
    inline void info    (string const& message) { os::log(Info   , message); }
    inline void warn    (string const& message) { os::log(Warn   , message); }
    inline void error   (string const& message) { os::log(Error  , message); }
}

FISSION_NAMESPACE_END

#if   defined(FISSION_PLATFORM_HEADLESS)
#   define FS_PLATFORM_NAME "Headless"
#   include "platform/headless.hpp"
#elif defined(FISSION_PLATFORM_WINDOWS)
#   define FS_PLATFORM_NAME "Windows"
#   include "platform/windows.hpp"
#elif defined(FISSION_PLATFORM_LINUX)
#   define FS_PLATFORM_NAME "Linux"
#   include "platform/linux.hpp"
#elif defined(FISSION_PLATFORM_ANDROID)
#   define FS_PLATFORM_NAME "Android"
#   include "platform/android.hpp"
#elif defined(FISSION_PLATFORM_MACOS)
#   define FS_PLATFORM_NAME "MacOS"
#   include "platform/macos.inl"
#elif defined(FISSION_PLATFORM_IOS)
#   define FS_PLATFORM_NAME "IOS"
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
