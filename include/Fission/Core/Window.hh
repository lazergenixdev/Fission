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
#include <Fission/Platform.hpp>
#include <Fission/Base/String.hpp>
#include <Fission/Base/Math/Vector.hpp>
#include <Fission/Core/Input/Event.hh>
#include <Fission/Core/Display.hh>
#include <iterator>
#include <vector>

__FISSION_BEGIN__

template <typename T, size_t S>
struct thread_safe_queue {
	// S is ignored for now, but I want to use this to
	//    limit the number events that can be queued.

	template <typename...A>
	inline void append(A&&... t) {
		std::scoped_lock lock(access_mutex);
		array.emplace_back(std::forward<A>(t)...);
	}

	inline void pop_all(std::vector<T>& out_array) {
		out_array.clear();
		std::scoped_lock lock(access_mutex);
		std::copy(array.begin(), array.end(), std::back_inserter(out_array));
		array.clear();
	}

	std::mutex access_mutex;
	std::vector<T> array;
};

using Event_Queue = thread_safe_queue<Event, 64>;

struct Window : public platform::Window_Impl
{
	Event_Queue event_queue;
	v2s32 mouse_position;
	int width, height;
	v2s32 position = {}; // position when in Windowed mode only
	Window_Mode mode;
	int display_index = Display_Index_Automatic;

public:
	void create(struct Window_Create_Info* info);

	FS_THREAD_SAFE void set_title(string const& title);
	FS_THREAD_SAFE void close();
	FS_THREAD_SAFE bool is_minimized();
	FS_THREAD_SAFE bool exists() const; // this function is weird
	
	//! @brief Display that is used is determined by the `display_index`
	FS_THREAD_SAFE void set_mode(Window_Mode mode);

	Window() = default;
	~Window();

#if defined(FISSION_PLATFORM_WINDOWS)
	LRESULT _win32_ProcessMessage(HWND, UINT, WPARAM, LPARAM);
	v2s32 _get_size();
	struct _Style { DWORD ex, value; };
	_Style _get_style();
#endif
}; // struct fs::Window

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