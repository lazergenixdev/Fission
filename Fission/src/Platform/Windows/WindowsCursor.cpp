#include "WindowsCursor.h"

namespace Fission::Platform
{
	namespace Cursors
	{
		static WindowsCursor Hidden		{ NULL };
		static WindowsCursor Arrow		{ LoadCursorW( NULL, IDC_ARROW ) };
		static WindowsCursor TextInput	{ LoadCursorW( NULL, IDC_IBEAM ) };
		static WindowsCursor Hand		{ LoadCursorW( NULL, IDC_HAND ) };
		static WindowsCursor Move		{ LoadCursorW( NULL, IDC_SIZEALL ) };
		static WindowsCursor Cross		{ LoadCursorW( NULL, IDC_CROSS ) };
		static WindowsCursor Wait		{ LoadCursorW( NULL, IDC_WAIT ) };
		static WindowsCursor SizeY		{ LoadCursorW( NULL, IDC_SIZENS ) };
		static WindowsCursor SizeX		{ LoadCursorW( NULL, IDC_SIZEWE ) };
		static WindowsCursor SizeBLTR	{ LoadCursorW( NULL, IDC_SIZENESW ) };
		static WindowsCursor SizeTLBR	{ LoadCursorW( NULL, IDC_SIZENWSE ) };
	}

	WindowsCursor::WindowsCursor( HCURSOR hCursor, bool shared )
		: handle( hCursor ), bShared( shared )
	{}

	WindowsCursor * WindowsCursor::Default( Cursor::Default_ def )
	{
		switch( def )
		{
		case Fission::Cursor::Default_Hidden:		return &Cursors::Hidden;
		case Fission::Cursor::Default_Arrow:		return &Cursors::Arrow;
		case Fission::Cursor::Default_TextInput:	return &Cursors::TextInput;
		case Fission::Cursor::Default_Hand:			return &Cursors::Hand;
		case Fission::Cursor::Default_Move:			return &Cursors::Move;
		case Fission::Cursor::Default_Cross:		return &Cursors::Cross;
		case Fission::Cursor::Default_Wait:			return &Cursors::Wait;
		case Fission::Cursor::Default_SizeY:		return &Cursors::SizeY;
		case Fission::Cursor::Default_SizeX:		return &Cursors::SizeX;
		case Fission::Cursor::Default_SizeBLTR:		return &Cursors::SizeBLTR;
		case Fission::Cursor::Default_SizeTLBR:		return &Cursors::SizeTLBR;
		default:									return nullptr;
		}
	}

	bool WindowsCursor::Use()
	{
		HCURSOR hPrev = GetCursor();
		return ( hPrev == SetCursor( handle ) );
	}

	WindowsCursor::~WindowsCursor()
	{
		if( !bShared )
			DestroyCursor( handle );
	}

}
