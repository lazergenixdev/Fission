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
#include <string>
__FISSION_BEGIN__

// Helping the compiler's dumb ass be more efficient:

// string from C string
#define FS_str(S)                 ::fs::string{.count = sizeof(S)-1, .data = (::fs::c8*)S}

// string from fixed buffer
#define FS_str_buffer(S)          ::fs::string{.count = sizeof(S), .data = (::fs::c8*)S}

// string from std::string (eww)
#define FS_str_std(S)             ::fs::string{.count = (S).size(), .data = (::fs::c8*)(S).data()}

// string from Pointer and Count
#define FS_str_make(PTR, COUNT)   ::fs::string{.count = static_cast<::fs::u64>(COUNT), .data = (::fs::c8*)PTR}

// Will this ever be useful?
#define FS_FORMAT_TO_STRING(SIZE, STR, FORMAT_STRING, ...) \
char FS_COMBINE2(__buffer_,__LINE__) [SIZE]; \
STR.count = sprintf_s(FS_COMBINE2(__buffer_,__LINE__), FORMAT_STRING, __VA_ARGS__); \
STR.data  = (::fs::c8*)FS_COMBINE2(__buffer_,__LINE__)

struct string {
	u64 count = 0;
	c8* data  = nullptr;

	inline constexpr std::string_view str() const noexcept {
		return std::string_view((char*)data, count);
	}

	inline constexpr string substr(u64 offset, u64 _count = 0xFFFFFFFF) {
		return string{.count = std::min(count - offset, _count), .data = data + offset};
	}
};

struct string_utf16 {
	u64 count = 0;
	c16* data = nullptr;
};

void convert_utf8_to_utf16(string_utf16* output_buffer, string       source); // out_size = in_size
void convert_utf16_to_utf8(string*       output_buffer, string_utf16 source); // out_size = in_size * 3

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