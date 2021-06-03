/**
*
* @file: Event.h
* @author: lazergenixdev@gmail.com
*
*
* This file is provided under the MIT License:
*
* Copyright (c) 2021 Lazergenix Software
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#pragma once
#include "Fission/config.h"
#include "Fission/Base/Math/Vector.h"
#include "Fission/Base/Size.h"
#include "Keys.h"
#include "Cursor.h"

/*! @brief Event has been handled */
#define FISSION_EVENT_HANDLED  ::Fission::EventResult::Handled

/*! @brief Event should be passed to the next event handler */
#define FISSION_EVENT_PASS     ::Fission::EventResult::Pass

/*! @brief Default behavior of event callbacks */
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
		bool repeat;
	};

	struct KeyUpEventArgs : public NativeEventArgs {
		Keys::Key key;
	};

	struct TextInputEventArgs : public NativeEventArgs {
		wchar_t character;
	};

	struct MouseMoveEventArgs : public NativeEventArgs {
		base::vector2i position;
	};

	struct MouseLeaveEventArgs : public NativeEventArgs {};

	struct SetCursorEventArgs : public NativeEventArgs {
		Cursor * cursor;
		bool bUseCursor = true; /*!< States whether the window should use the cursor argument when event handled */
	};

	struct ResizeEventArgs : public NativeEventArgs {
		base::size size; 
	};



/* ================================================================================================== */
/* -------------------------------------- Event Handler --------------------------------------------- */
/* ================================================================================================== */
	
	fission_Interface IEventHandler
	{

		/****************************************************************************************
		* @brief 
		*   Get the Default Event Handler (does not respond to events).
		* 
		* @note:   
		*   Allows for the ability to always call the event handler with no nullptr errors.
		*   Use `IEventHandler::Default()` instead of `nullptr`.
		*/
		FISSION_API static IEventHandler * Default();



		FISSION_THREAD_SAFE 
		virtual EventResult OnKeyDown( KeyDownEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE 
		virtual EventResult OnKeyUp( KeyUpEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE 
		virtual EventResult OnTextInput( TextInputEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE
		virtual EventResult OnMouseMove( MouseMoveEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE
		virtual EventResult OnMouseLeave( MouseLeaveEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE
		virtual EventResult OnSetCursor( SetCursorEventArgs & )
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE
		virtual EventResult OnHide()
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE 
		virtual EventResult OnShow()
		FISSION_EVENT_DEFAULT

		FISSION_THREAD_SAFE
		virtual EventResult OnClose( CloseEventArgs & )
		FISSION_EVENT_DEFAULT

	//	FISSION_THREAD_SAFE
	//	virtual EventResult OnResize( ResizeEventArgs & )
	//	FISSION_EVENT_DEFAULT

	}; // struct Fission::IEventHandler

} // namespace Fission
