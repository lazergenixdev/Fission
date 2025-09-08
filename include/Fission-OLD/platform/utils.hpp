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
#include <Fission/platform.hpp>
#include <filesystem>

namespace os
{
    struct scoped_lock
    {
        Mutex mutex;

        scoped_lock(Mutex in_mutex): mutex(in_mutex) {
            if (os_mutex_lock(mutex))
                fs::log::error("(scoped_lock) Failed to lock mutex!");
        }

        ~scoped_lock() {
            if (os_mutex_unlock(mutex))
                fs::log::error("(scoped_lock) Failed to unlock mutex!");
        }
    };
}

#if 0
namespace platform
{
	using namespace std::filesystem;

	// returns nullptr if file does not exist or unable to load.
	FISSION_API void* load_entire_file(path const& _File_Path, u64* out_file_size);

	// returns true on failure
	FISSION_API bool dump_to_file(path const& _File_Path, void* data, u64 size);

	FISSION_API bool open_url(path const& _URL);
	FISSION_API bool open_file_location(path const& _File);

	FISSION_API path open_file_dialog(char const* _Name, char const* _Extensions);
}
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