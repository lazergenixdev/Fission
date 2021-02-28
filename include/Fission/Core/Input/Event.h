#pragma once
#include "Fission/config.h"
#include "Keys.h"
#include "Cursor.h"

#define FISSION_EVENT_HANDLED  ::Fission::EventResult::Handled
#define FISSION_EVENT_PASS     ::Fission::EventResult::Pass

#define FISSION_EVENT_DEFAULT { return FISSION_EVENT_PASS; }

namespace Fission {

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
		// this is some next level formatting right here

		FISSION_THREAD_SAFE virtual EventResult OnKeyDown
		( KeyDownEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE virtual EventResult OnKeyUp
		( KeyUpEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE virtual EventResult OnTextInput
		( TextInputEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE virtual EventResult OnMouseMove
		( MouseMoveEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE virtual EventResult OnMouseLeave
		( MouseLeaveEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE virtual EventResult OnSetCursor
		( SetCursorEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE virtual EventResult OnHide
		()
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE virtual EventResult OnShow
		()
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE virtual EventResult OnClose
		( CloseEventArgs &	)
		FISSION_EVENT_DEFAULT

	//	FISSION_THREAD_SAFE virtual EventResult OnResize
	//  ( ResizeEventArgs & )
	//	FISSION_EVENT_DEFAULT

	public:
		// Returns a null EventHandler(does not respond to events), Use in place of 'nullptr'
		// Allows for the ability to always call the event handler with no nullptr errors
		FISSION_API static IEventHandler * Default();

	}; // interface Fission::IEventHandler

} // namespace Fission
