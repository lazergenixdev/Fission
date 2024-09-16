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
// todo: add more keys

#pragma once
#include <Fission/config.hpp>
#include <Fission/Platform.hpp>

__FISSION_BEGIN__

#define FISSION_X_24              \
X(1)X(2)X(3)X(4)X(5)X(6)X(7)X(8)  \
X(9)X(10)X(11)X(12)X(13)X(14)     \
X(15)X(16)X(17)X(18)X(19)X(20)    \
X(21)X(22)X(23)X(24)

namespace keys
{
	enum Key_Id : u32
	{
		Unknown = 0,

#if defined(FISSION_PLATFORM_WINDOWS)
#   define KEY(WIN32_KEY,_0) WIN32_KEY
#elif defined(FISSION_PLATFORM_LINUX)
#   define KEY(_0,LINUX_KEY) LINUX_KEY
#else
#   error "Don't know the keys for this platform"
#endif
		Mouse_Left      = KEY(VK_LBUTTON, 0),
		Mouse_Right     = KEY(VK_RBUTTON, 1),
		Mouse_Middle    = KEY(VK_MBUTTON, 2),
		Mouse_WheelUp   = KEY(0xF9, 3),
		Mouse_WheelDown = KEY(0xFF, 4),

		/* Gamepad */
	//	Gamepad_Y,
	//	Gamepad_X,
	//	Gamepad_A,
	//	Gamepad_B,
	//	Gamepad_Up,
	//	Gamepad_Left,
	//	Gamepad_Right,
	//	Gamepad_Down,
	//	Gamepad_Start,
	//	Gamepad_Home,
	//	Gamepad_RBumper,
	//	Gamepad_LBumper,

#define X(N) F ## N = KEY(VK_F ## N, 65469+N),
		FISSION_X_24
#undef X
#define X(N) \
Num    ## N = KEY(#N[0], 33+N), \
Numpad ## N = KEY(VK_NUMPAD ## N, 43+N),
		FISSION_X_BASE10
#undef X
#define X(L) L = KEY(#L[0], 32+#L[0]),
		FISSION_X_LETTERS
#undef X

		Up       = KEY(VK_UP     , 65362),
		Down     = KEY(VK_DOWN   , 65364),
		Left     = KEY(VK_LEFT   , 65361),
		Right    = KEY(VK_RIGHT  , 65363),

		Space    = KEY(VK_SPACE  , 32),
		Escape   = KEY(VK_ESCAPE , 75),
		Shift    = KEY(VK_SHIFT  , 76),
		Control  = KEY(VK_CONTROL, 77),
		Alt      = KEY(VK_MENU   , 78),
		Tab      = KEY(VK_TAB    , 79),
		Capital  = KEY(VK_CAPITAL, 80),
		Back     = KEY(VK_BACK   , 81),
		Delete   = KEY(VK_DELETE , 82),
		Enter    = KEY(VK_RETURN , 83),

		Accent     = KEY(VK_OEM_3, 84), // `
		Comma,			       // ,
		Period,			       // .
		FSlash,			       // /
		BSlash,			       // |
		Semicolon,		       // ;
		Apostrophe,		       // '
		LBracket,		       // [
		RBracket,		       // ]
		Minus,			       // -
		Equal,			       // =
                               //
	}; // Fission::Keys::Key

	enum Modifier : u64 {
		Mod_LShift   = 1 << 0,
		Mod_RShift   = 1 << 1,
		Mod_LControl = 1 << 2,
		Mod_RControl = 1 << 3,
		Mod_LAlt     = 1 << 4,
		Mod_RAlt     = 1 << 5,

		Mod_Shift   = Mod_LShift   | Mod_RShift,
		Mod_Control = Mod_LControl | Mod_RControl,
		Mod_Alt     = Mod_LAlt     | Mod_RAlt,
	};

	//! @warning NOT IMPLEMENTED
	static constexpr Key_Id string_to_key( const char * _str )
	{
		return Key_Id::Unknown;
	}

	constexpr const char * key_to_string(Key_Id key) {
		switch( key )
		{
		case Mouse_Left: 		return "Left Mouse";
		case Mouse_Right:		return "Left Mouse";
		case Mouse_Middle:		return "Middle Mouse";
//		case Mouse_WheelUp:		return "Mouse Wheel Up";
//		case Mouse_WheelDown:	return "Mouse Wheel Down";
//		case Gamepad_X:			return "Gamepad X";
//		case Gamepad_Y:			return "Gamepad Y";
//		case Gamepad_A:			return "Gamepad A";
//		case Gamepad_B:			return "Gamepad B";
//		case Gamepad_Up:		return "Gamepad Up";
//		case Gamepad_Left:		return "Gamepad Left";
//		case Gamepad_Right:		return "Gamepad Right";
//		case Gamepad_Down:		return "Gamepad Down";
//		case Gamepad_Start:		return "Gamepad Start";
//		case Gamepad_Home:		return "Gamepad Home";
//		case Gamepad_RBumper:	return "Gamepad Right Bumper";
//		case Gamepad_LBumper:	return "Gamepad Left Bumper";
		case F1:				return "F1";
		case F2:				return "F2";
		case F3:				return "F3";
		case F4:				return "F4";
		case F5:				return "F5";
		case F6:				return "F6";
		case F7:				return "F7";
		case F8:				return "F8";
		case F9:				return "F9";
		case F10:				return "F10";
		case F11:				return "F11";
		case F12:				return "F12";
		case Shift:				return "Shift";
		case Control:			return "Control";
		case Alt:				return "Alt";
		case Tab:				return "Tab";
		case Capital:			return "Capital";
		case Escape:			return "Escape";
		case Back:				return "Backspace";
		case Delete:			return "Delete";
		case Enter:				return "Enter";
		case Space:				return "Space";
		case Num0:				return "0";
		case Num1:				return "1";
		case Num2:				return "2";
		case Num3:				return "3";
		case Num4:				return "4";
		case Num5:				return "5";
		case Num6:				return "6";
		case Num7:				return "7";
		case Num8:				return "8";
		case Num9:				return "9";
		case A:					return "A";
		case B:					return "B";
		case C:					return "C";
		case D:					return "D";
		case E:					return "E";
		case F:					return "F";
		case G:					return "G";
		case H:					return "H";
		case I:					return "I";
		case J:					return "J";
		case K:					return "K";
		case L:					return "L";
		case M:					return "M";
		case N:					return "N";
		case O:					return "O";
		case P:					return "P";
		case Q:					return "Q";
		case R:					return "R";
		case S:					return "S";
		case T:					return "T";
		case U:					return "U";
		case V:					return "V";
		case W:					return "W";
		case X:					return "X";
		case Y:					return "Y";
		case Z:					return "Z";
		case Up:				return "Up";
		case Down:				return "Down";
		case Left:				return "Left";
		case Right:				return "Right";
		case Accent:			return "`";
		case Comma:				return ",";
		case Period:			return ".";
		case FSlash:			return "/";
		case BSlash:			return "\\";
		case Semicolon:			return ";";
		case Apostrophe:		return "'";
		case LBracket:			return "[";
		case RBracket:			return "]";
		case Minus:				return "-";
		case Equal:				return "=";
		case Numpad0:			return "Numpad 0";
		case Numpad1:			return "Numpad 1";
		case Numpad2:			return "Numpad 2";
		case Numpad3:			return "Numpad 3";
		case Numpad4:			return "Numpad 4";
		case Numpad5:			return "Numpad 5";
		case Numpad6:			return "Numpad 6";
		case Numpad7:			return "Numpad 7";
		case Numpad8:			return "Numpad 8";
		case Numpad9:			return "Numpad 9";
		default:				return "Unknown";
		}
	}

} // namespace fs::keys

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
