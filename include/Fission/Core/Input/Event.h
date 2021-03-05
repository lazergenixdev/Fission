#pragma once
#include "Fission/config.h"
#include "Keys.h"
#include "Cursor.h"

/*!< Event has been handled */
#define FISSION_EVENT_HANDLED  ::Fission::EventResult::Handled

/*!< Event should be passed to the next event handler */
#define FISSION_EVENT_PASS     ::Fission::EventResult::Pass

/*!< Default behavior of event callbacks */
#define FISSION_EVENT_DEFAULT { return FISSION_EVENT_PASS; }


namespace Fission {

	//! @brief Result of a Event Callback
	enum class EventResult
	{
		Handled = 0,
		Pass = 1,
	};


/* -------------------------------------- Event Arguments ------------------------------------------- */

	struct NativeEventArgs {
		Platform::Event * native_event;
	};

	struct CloseEventArgs {
		int ExitCode; 
	};

	struct KeyDownEventArgs : public NativeEventArgs {
		Keys::Key key;
	};

	struct KeyUpEventArgs : public NativeEventArgs {
		Keys::Key key;
	};

	struct TextInputEventArgs : public NativeEventArgs {
		wchar_t character;
	};

	struct MouseMoveEventArgs : public NativeEventArgs {
		vec2i position;
	};

	struct MouseLeaveEventArgs : public NativeEventArgs {};

	struct SetCursorEventArgs : public NativeEventArgs {
		Cursor * cursor;
	};

	struct ResizeEventArgs : public NativeEventArgs {
		vec2i size; 
	};



/* ================================================================================================== */
/* -------------------------------------- Event Handler --------------------------------------------- */
/* ================================================================================================== */
	
	struct IEventHandler
	{

		/****************************************************************************************
		* @brief 
		*   Get the Default Event Handler.
		* 
		* @note:   
		*   Allows for the ability to always call the event handler with no nullptr errors.
		*   Use this instead of `nullptr`.
		* 
		* @return null EventHandler (does not respond to events).
		*/
		FISSION_API static IEventHandler * Default();



		// this is some next level formatting right here:

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

	}; // struct Fission::IEventHandler

} // namespace Fission
