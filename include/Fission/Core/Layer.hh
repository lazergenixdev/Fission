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
#include <Fission/Base/String.hpp>
#include <Fission/Core/Input/Event.hh>
#include <vector>

__FISSION_BEGIN__

namespace layer {
	enum Flags: u32 {
		show   = 1 << 0,
		enable = 1 << 1,
	};
}

struct Debug_Layer {
	// note: string is copied, no need to keep the string memory around :)
	void add(string s);

	template <size_t Buffer_Size = 128, typename...T>
	void add(const char* fmt, T&&...args)
	{
		char _buffer[Buffer_Size];
		size_t count = sprintf_s(_buffer, fmt, std::forward<T>(args)...);
		add(FS_str_make(_buffer, count));
	}

	u32 flags = 0;

	float* frame_times;
	int frame_count;
	int frame_time_index = 0;
	
	// TODO: Use string "ranges" instead of "string", use after free error waiting to happen...
	string app_info_string;
	std::vector<string> right_strings;
	std::vector<string> left_strings;
	std::vector<char>   character_buffer;
	int character_count = 0;

	void handle_events(std::vector<struct Event>& events);
	void on_update(double dt, struct Render_Context* ctx);
	
	void create();
	void destroy();
};

// ESCXXXhelloESC
// X for a byte [0,255] representing a color component
// XXX => RGB
struct Console_Layer {
	u32 flags = 0;
	
	static constexpr u64 default_buffer_size = FS_KILOBYTES(4);

	float current;
	string input;
	c8 input_buffer[72];
	string buffer_view;
	u64 buffer_capacity;

	void handle_events(std::vector<struct Event>& events);
	void on_update(double dt, struct Render_Context* ctx);

	void create();
	void destroy();

	void _reserve_space_for(u64 added_count);
private:
	void draw_console_buffer(struct Textured_Renderer_2D& r, float top, float ystride);
	void handle_character_input(Event::Character_Input in);

	static constexpr u64 minimum_buffer_delete_count = FS_KILOBYTES(1);
	static constexpr u64 maximum_buffer_delete_count = FS_KILOBYTES(2);
};

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