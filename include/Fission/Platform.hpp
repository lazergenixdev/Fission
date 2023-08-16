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

#if defined(FISSION_PLATFORM_WINDOWS)
#include <Fission/Platform/Windows/winapi.h>
#include <thread>
#include <mutex>
#include <condition_variable>
__FISSION_BEGIN__

#ifndef FS_utf16_codepoint_builder_error
#define FS_utf16_codepoint_builder_error(EXPR, MSG) (void)0
#endif

struct UTF_16_Codepoint_Builder {
public:
    c32 codepoint = 0;

    // returns true iff codepoint is now built.
    // note: garbage in -> garbage out
    bool append(c16 utf16) {
        // boring case
        if (utf16 < 0xD800) {
            FS_utf16_codepoint_builder_error(bytes_occupied != 0, "Invalid continuation character");

            codepoint = utf16;
            return true;
        }

        switch (slots_occupied)
        {
        case 0:
            // store first half of codepoint.
            codepoint = (utf16 - 0xD800) * 0x400;
            break;
        case 1:
            // store the second half of codepoint.
            codepoint = ((utf16 - 0xDC00) + codepoint + 0x10000);
            break;
        default:
            FS_utf16_codepoint_builder_error(true, "Slots Occupied cannot be any value other than 0 or 1");
            break;
        }

        ++slots_occupied;

        if (slots_occupied == 2) {
            slots_occupied = 0;
            return true;
        }

        return false;
    }

private:
    // every "slot" is two bytes
    int slots_occupied = 0;
};

namespace platform {
	struct Instance {}; // useless on windows
    enum Window_ {
        Window_Disable_Position_Update = 1 << 0,
    };
	struct Window_Impl {
		HWND                     _handle = NULL;
		std::thread              _thread;
		std::mutex               _mutex;
		std::condition_variable  _cv;
		short                    _mouse_wheel_delta;
		UTF_16_Codepoint_Builder _codepoint_builder;
        u32                      _flags = 0;
	};
	struct Display_Impl {
		HMONITOR _handle;
	};
}
__FISSION_END__
#endif

/**
 *	MIT License
 *
 *	Copyright (c) 2021-2023 lazergenixdev
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