#pragma once
#include "Fission/config.h"

namespace Fission
{
	namespace Keys
	{
		enum Key : char
		{
			Unknown = 0,

			/* Mouse buttons */
			Mouse_Left,
			Mouse_Right,
			Mouse_Middle,
			Mouse_WheelUp,
			Mouse_WheelDown,

			__Mouse_Button_Start = Mouse_Left,
			__Mouse_Button_End = Mouse_WheelDown,

			/* Gamepad */
			Gamepad_X,
			Gamepad_Y,
			Gamepad_A,
			Gamepad_B,
			Gamepad_Up,
			Gamepad_Left,
			Gamepad_Right,
			Gamepad_Down,
			Gamepad_Start,
			Gamepad_Home,
			Gamepad_RBumper,
			Gamepad_LBumper,

			__Gamepad_Start = Gamepad_X,
			__Gamepad_End = Gamepad_LBumper,

			/* Keyboard */
			F1,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,

			Shift,
			Control,
			Alt,
			Tab,
			Capital,
			Escape,
			Back,
			Delete,

			Space,

			Num0,
			Num1,
			Num2,
			Num3,
			Num4,
			Num5,
			Num6,
			Num7,
			Num8,
			Num9,

			A,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			M,
			N,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,

			Up,
			Down,
			Left,
			Right,

			Accent,			// `
			Comma,			// ,
			Period,			// .
			FSlash,			// /
			BSlash,			// |
			Semicolon,		// ;
			Apostrophe,		// '
			LBracket,		// [
			RBracket,		// ]
			Minus,			// -
			Equal,			// =

			Numpad0,
			Numpad1,
			Numpad2,
			Numpad3,
			Numpad4,
			Numpad5,
			Numpad6,
			Numpad7,
			Numpad8,
			Numpad9,

			__Keyboard_Start = F1,
			__Keyboard_End = Numpad9,

			__count__,
		}; // Fission::Keys::Key

		//! @warning NOT IMPLEMENTED
		static constexpr Key string_to_key( const char * _str )
		{
			return Keys::Unknown;
		}

		constexpr const char * key_to_string( Key key ) {
			switch( key )
			{
			case Mouse_Left: 		return "Left Mouse";
			case Mouse_Right:		return "Left Mouse";
			case Mouse_Middle:		return "Middle Mouse";
			case Mouse_WheelUp:		return "Mouse Wheel Up";
			case Mouse_WheelDown:	return "Mouse Wheel Down";
			case Gamepad_X:			return "Gamepad X";
			case Gamepad_Y:			return "Gamepad Y";
			case Gamepad_A:			return "Gamepad A";
			case Gamepad_B:			return "Gamepad B";
			case Gamepad_Up:		return "Gamepad Up";
			case Gamepad_Left:		return "Gamepad Left";
			case Gamepad_Right:		return "Gamepad Right";
			case Gamepad_Down:		return "Gamepad Down";
			case Gamepad_Start:		return "Gamepad Start";
			case Gamepad_Home:		return "Gamepad Home";
			case Gamepad_RBumper:	return "Gamepad Right Bumper";
			case Gamepad_LBumper:	return "Gamepad Left Bumper";
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

		static constexpr bool is_keyboard( Key key ) {
			return ( __Keyboard_Start <= key ) && ( key <= __Keyboard_End );
		}

		static constexpr bool is_mouse( Key key ) {
			return ( __Mouse_Button_Start <= key ) && ( key <= __Mouse_Button_End );
		}

		static constexpr bool is_gamepad( Key key ) {
			return ( __Gamepad_Start <= key ) && ( key <= __Gamepad_End );
		}
	} // namespace Fission::Keys

	namespace Platform 
	{
#ifdef FISSION_PLATFORM_WINDOWS
		static constexpr Keys::Key key_from_win32( int key_code ) {
			switch( key_code ) {

				case VK_F1:				return Keys::F1;
				case VK_F2:				return Keys::F2;
				case VK_F3:				return Keys::F3;
				case VK_F4:				return Keys::F4;
				case VK_F5:				return Keys::F5;
				case VK_F6:				return Keys::F6;
				case VK_F7:				return Keys::F7;
				case VK_F8:				return Keys::F8;
				case VK_F9:				return Keys::F9;
				case VK_F10:			return Keys::F10;
				case VK_F11:			return Keys::F11;
				case VK_F12:			return Keys::F12;
										
				case VK_SPACE:			return Keys::Space;
				case VK_ESCAPE:			return Keys::Escape;
				case VK_SHIFT:			return Keys::Shift;
				case VK_CONTROL:		return Keys::Control;
				case VK_MENU:			return Keys::Alt;
				case VK_TAB:			return Keys::Tab;
				case VK_CAPITAL:		return Keys::Capital;
				case VK_BACK:			return Keys::Back;
				case VK_DELETE:			return Keys::Delete;
										
				case VK_UP:				return Keys::Up;
				case VK_DOWN:			return Keys::Down;
				case VK_LEFT:			return Keys::Left;
				case VK_RIGHT:			return Keys::Right;
				case VK_OEM_3:			return Keys::Accent;

				case 'A':				return Keys::A;
				case 'B':				return Keys::B;
				case 'C':				return Keys::C;
				case 'D':				return Keys::D;
				case 'E':				return Keys::E;
				case 'F':				return Keys::F;
				case 'G':				return Keys::G;
				case 'H':				return Keys::H;
				case 'I':				return Keys::I;
				case 'J':				return Keys::J;
				case 'K':				return Keys::K;
				case 'L':				return Keys::L;
				case 'M':				return Keys::M;
				case 'N':				return Keys::N;
				case 'O':				return Keys::O;
				case 'P':				return Keys::P;
				case 'Q':				return Keys::Q;
				case 'R':				return Keys::R;
				case 'S':				return Keys::S;
				case 'T':				return Keys::T;
				case 'U':				return Keys::U;
				case 'V':				return Keys::V;
				case 'W':				return Keys::W;
				case 'X':				return Keys::X;
				case 'Y':				return Keys::Y;
				case 'Z':				return Keys::Z;

				case '1':				return Keys::Num1;
				case '2':				return Keys::Num2;
				case '3':				return Keys::Num3;
				case '4':				return Keys::Num4;
				case '5':				return Keys::Num5;
				case '6':				return Keys::Num6;
				case '7':				return Keys::Num7;
				case '8':				return Keys::Num8;
				case '9':				return Keys::Num9;

				case VK_NUMPAD0:		return Keys::Numpad0;
				case VK_NUMPAD1:		return Keys::Numpad1;
				case VK_NUMPAD2:		return Keys::Numpad2;
				case VK_NUMPAD3:		return Keys::Numpad3;
				case VK_NUMPAD4:		return Keys::Numpad4;
				case VK_NUMPAD5:		return Keys::Numpad5;
				case VK_NUMPAD6:		return Keys::Numpad6;
				case VK_NUMPAD7:		return Keys::Numpad7;
				case VK_NUMPAD8:		return Keys::Numpad8;
				case VK_NUMPAD9:		return Keys::Numpad9;

				default:				return Keys::Unknown;

			}
		}
#endif // FISSION_PLATFORM_WINDOWS

	//	extern  Keys::Key key_from_andriod( int key_code );

	} // namespace Fission::Platform

} // namespace Fission