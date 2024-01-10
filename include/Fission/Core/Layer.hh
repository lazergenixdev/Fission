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
#include <Fission/Base/Math/Vector.hpp>
#include <Fission/Core/Input/Event.hh>
#include <vector>

__FISSION_BEGIN__

// @note: this is better than using stupid `enum class`;
//         cannot do `using` with enum classes!
namespace layer {
	enum Flags: u32 {
		show   = 1 << 0,
		enable = 1 << 1,

		debug_show_verbose    = 1 << 2,
		console_end_of_buffer = 1 << 2, // flag set if cannot scroll up any further
	};
}

template <typename Layer>
static inline constexpr void disable_layer(Layer& layer) noexcept {
	layer.flags &=~ (layer::show | layer::enable);
}
template <typename Layer>
static inline constexpr void enable_layer(Layer& layer) noexcept {
	layer.flags |= layer::enable;
}

struct Debug_Layer {
	// note: string is copied, no need to keep the string memory around :)
	void add(string s);

	template <size_t Buffer_Size = 64, typename...T>
	void add(const char* fmt, T&&...args)
	{
		char buffer[Buffer_Size] = {};
		size_t count = snprintf(buffer, Buffer_Size, fmt, std::forward<T>(args)...);
		add(FS_str_make(buffer, count));
	}

	u32 flags = layer::enable | layer::debug_show_verbose;

	float* frame_times;
	int frame_count;
	int frame_time_index = 0;

	float cpu_time = 0.0f;
	
	string_view app_info_string;
	std::vector<string_view> right_strings;
	std::vector<string_view> left_strings;
	std::vector<c8>          character_buffer;
	int character_count_initial = 0;

	void handle_events(std::vector<struct Event>& events);
	void on_update(double dt, struct Render_Context* ctx);
	
	void create();
	void destroy();

private:
	float draw_frame_time_graph(v2f32 top_left);
	bool visible() const;
};

// ESCXXXhelloESC
// X for a byte [0,255] representing a color component
// XXX => RGB
struct Console_Layer {
	u32 flags = layer::enable;
	
	static constexpr u64 default_buffer_size = FS_KILOBYTES(4);

	float    position;

	string   input; // input from user
	c8       input_buffer[72];
	int      input_cursor = 2;

	string   buffer_view; // fixed size console buffer to display stuff
	u64      buffer_capacity;
	int      buffer_view_offset = 0;

	s64              current_command = -1; // -1 == "user input", [0, command_count-1] == "command in command_history_buffer"
	std::vector<u32> command_history_ends; // constains the end indicies for all commands in command_history_buffer
	std::vector<c8>  command_history_buffer;

	void handle_events(std::vector<struct Event>& events);
	void on_update(double dt, struct Render_Context* ctx);

	void setup_console_api();

	void create();
	void destroy();

	void _reserve_space_for(u64 added_count);
private:
	void draw_console_buffer(struct Textured_Renderer_2D& r, float top, float ystride);
	void handle_character_input(Event::Character_Input in);
	string command_from_history();

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