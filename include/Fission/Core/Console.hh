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
#include <Fission/Base/String.hpp>
#include <Fission/Base/Color.hpp>

__FISSION_BEGIN__

namespace console_callback {
	void procedure(string args);

// Maybe add user data pointer? could be useful in reusing functions
	void __procedure(void* user, string args);
}

using console_callback_proc = decltype(&console_callback::procedure);

namespace console {
	FISSION_API void println(string text);
	FISSION_API void println(string text, rgb8 color);

	FISSION_API void print(string text);
	FISSION_API void print(string text, rgb8 color);

	FISSION_API void clear();

	FISSION_API void register_command(string name, console_callback_proc proc);
	FISSION_API void unregister_command(string name);
	
	template <int buffer_size = 128, typename...T>
	static void printf(const char* format, T&&...args) {
		char buffer[buffer_size];
		string formatted;
		formatted.count = sprintf(buffer, format, std::forward<T>(args)...);
		formatted.data = (::fs::c8*)buffer;
		print(formatted);
	}
}

__FISSION_END__

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