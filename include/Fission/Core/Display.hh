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
#include <Fission/Platform.hpp>
#include <Fission/Base/Math/Vector.hpp>
#include <Fission/Base/Rect.hpp>
#include <vector>

__FISSION_BEGIN__

enum Window_Mode: u32 {
	Windowed             = 0,
	Windowed_Fullscreen  = 1,
	Exclusive_Fullscreen = 2,
};

struct Display_Mode {
	v2u32 resolution;
	int   refresh_rate;
};

enum Display_Index_: int
{
	Display_Index_Primary = 0,

	//! @brief Set in a window's properties for it to determine
	//!        it's monitor automatically based on where the window is.
	Display_Index_Automatic = -1,
};

struct Display : public platform::Display_Impl
{
	int index;
	c8 name_buffer[64];
	int name_count;
	rs32 rect;

	string name() const noexcept { return FS_str_make(name_buffer, name_count); }

	Display_Mode current_mode() const;

	/*
	std::vector<Display_Mode> supported_display_modes();
	bool set_display_mode(const Display_Mode *);
	bool revert_display_mode();
	*/
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