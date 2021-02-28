#pragma once
#include "LazerEngine/config.h"
#include "Keys.h"
#include "Cursor.h"

#define LAZER_EVENT_HANDLED	::lazer::EventResult::Handled
#define LAZER_EVENT_PASS	::lazer::EventResult::Pass

// reminder that these functions can introduce race conditions
#define LAZER_THREAD_SAFE

#define LAZER_EVENT_DEFAULT { return LAZER_EVENT_PASS; }

namespace lazer {

	enum class EventResult : int
	{
		Handled = 0,
		Pass = 1,
	};

/* ================================================================================================== */
/* -------------------------------------- Event Arguments ------------------------------------------- */
/* ================================================================================================== */

	struct CloseEventArgs 
	{
		int ExitCode; 
	};

	struct KeyDownEventArgs
	{
		Keys::Key key;
	};

	struct KeyUpEventArgs
	{
		Keys::Key key;
	};

	struct TextInputEventArgs
	{
		wchar_t character;
	};

	struct MouseMoveEventArgs
	{
		vec2i position;
	};

	struct MouseLeaveEventArgs 
	{
	};

	struct SetCursorEventArgs
	{
		Cursor * cursor;
	};

	struct ResizeEventArgs
	{
		vec2i size; 
	};


/* ================================================================================================== */
/* -------------------------------------- Event Handler --------------------------------------------- */
/* ================================================================================================== */
	
	interface IEventHandler
	{

		LAZER_THREAD_SAFE virtual EventResult OnKeyDown( KeyDownEventArgs & )
		LAZER_EVENT_DEFAULT

		LAZER_THREAD_SAFE virtual EventResult OnKeyUp( KeyUpEventArgs & )
		LAZER_EVENT_DEFAULT

		LAZER_THREAD_SAFE virtual EventResult OnTextInput( TextInputEventArgs & )
		LAZER_EVENT_DEFAULT

		LAZER_THREAD_SAFE virtual EventResult OnMouseMove( MouseMoveEventArgs & )
		LAZER_EVENT_DEFAULT

		LAZER_THREAD_SAFE virtual EventResult OnMouseLeave( MouseLeaveEventArgs & )
		LAZER_EVENT_DEFAULT

		LAZER_THREAD_SAFE virtual EventResult OnSetCursor( SetCursorEventArgs & )
		LAZER_EVENT_DEFAULT

		LAZER_THREAD_SAFE virtual EventResult OnHide()
		LAZER_EVENT_DEFAULT

		LAZER_THREAD_SAFE virtual EventResult OnShow()
		LAZER_EVENT_DEFAULT

		LAZER_THREAD_SAFE virtual EventResult OnClose( CloseEventArgs &	)
		LAZER_EVENT_DEFAULT

	//	LAZER_THREAD_SAFE virtual EventResult OnResize ( ResizeEventArgs & )
	//	LAZER_EVENT_DEFAULT

	public:
		// Returns a null EventHandler(does not respond to events), Use in place of 'nullptr'
		// Allows for the ability to always call the event handler with no nullptr errors
		LAZER_API static IEventHandler * Default();

	};

}
