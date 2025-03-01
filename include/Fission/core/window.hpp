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
#include <Fission/base/string.hpp>
#include <Fission/base/math/vector.hpp>
#include <Fission/core/input/event.hpp>
#include <Fission/core/display.hpp>
#include <Fission/platform/utils.hpp>
#include <mutex>
#include <vector>

FISSION_NAMESPACE_BEGIN

enum Window_Mode: u32 {
	Windowed             = 0x01,
	Windowed_Fullscreen  = 0x02,
	Exclusive_Fullscreen = 0x04,
	Windowed_Resizeable  = 0x08,
};

template <typename T, size_t S>
struct thread_safe_queue {
	// S is ignored for now, but I want to use this to
	//    limit the number events that can be queued.

	thread_safe_queue() {
		os_mutex_create(&access_mutex);
        array.reserve(1000);
	}

	~thread_safe_queue() {
		os_mutex_destroy(access_mutex);
	}

	inline void append(T const& event) {
		os::scoped_lock lock {access_mutex};
		array.emplace_back(event);
	}

	inline void pop_all(std::vector<T>& out_array) {
		out_array.clear();
        os::scoped_lock lock {access_mutex};
        for (auto&& item: array)
            out_array.emplace_back(item);
	    array.clear();
    }

	os::Mutex access_mutex;
	std::vector<T> array;
};

using Event_Queue = thread_safe_queue<Event, 64>;

struct Window : public platform::Window
{
	Event_Queue  event_queue      {};
	v2s32        mouse_position   {};
	Window_Mode  mode             {Windowed_Fullscreen};
	int          display_index    {Display_Index_Automatic};
	bool         use_mouse_deltas {false};

	// Private API
	auto create(struct Window_Create_Info const& info) -> bool;

	// Private API
	// When a window is created, it will be hidden, calling
	//  this will show the window (at least in Windows).
    void show();

	// Private API
	// Closes the window => causes engine to stop running
	// => Application closes
	// Don't call this function, use Engine::flags and
	//  set the Running Bit to zero.
	void close();

	// @see enum Window_Mode
	auto supported_modes() -> u32;

	//void set_title(string const& title);
	//bool is_minimized();
	//void set_mode(Window_Mode mode);

	inline void toggle_using_mouse_deltas() {
    	set_using_mouse_delta(!use_mouse_deltas);
	}

	void set_using_mouse_delta(bool use);

	~Window();
};

FISSION_NAMESPACE_END

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
