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

__FISSION_BEGIN__

//// functions that do not exist
//void on_key_down(u32 key_id, bool repeat);
//void on_key_up(u32 key_id);
//
//struct WindowCallbacks {
//	decltype(&on_key_down) key_down;
//	decltype(&on_key_up)   key_up;
//};

enum EventType {
	Event_Key_Down,
	Event_Key_Up,
	Event_Focus_Lost,
	Event_Character_Input,

	// Does this really need to be here?
	// mouse position is always stored in `engine.window.mouse_position`
	Event_Mouse_Move_Absolute,

	Event_Mouse_Move_Relative,
};

struct Event {
	s64 timestamp;
	u8 type;

	struct Key_Down {
		u32 key_id;
	};
	struct Key_Up {
		u32 key_id;
	};
	struct Character_Input {
		c32 codepoint;
	};
	struct Mouse_Move_Absolute {
		v2s32 position;
	};
	struct Mouse_Move_Relative {
		v2s32 delta;
	};

	union {
		Key_Down            key_down;
		Key_Up              key_up;
		Character_Input     character_input;
		Mouse_Move_Absolute mouse_move_absolute;
		Mouse_Move_Relative mouse_move_relative;
	};
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