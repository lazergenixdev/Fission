/**
*
* @file: Window.h
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
#include "Input/Event.h"
#include "lazer/utility.h"
#include "Fission/Core/Monitor.h"
#include "Fission/Core/Graphics/Graphics.h"

// todo: implement fixed aspect-ratio flag
// todo: implement fullscreen (will need a monitor API to pick what screen to fullscreen on)

namespace Fission
{
	class Window
	{
	public:

		/*! @brief Native window handle type */
		using native_handle_type = Platform::WindowHandle;

		using SaveID = int;

		static constexpr SaveID NoSaveID = -1; /*!< Window properties will not be saved. */


		enum class Style
		{
			Borderless,			// Window has no border or title bar
			Border,				// Window has a border around it with a close button
			BorderSizeable,		// Window how a border with min/max and able to be resized
			Fullscreen,			// Window is fullscreen

			Default = Border
		};

		enum class Flags : utility::flag
		{
			None					= 0,
			RestrictAspectRatio		= utility::make_flag<0>, /* Restrict the window to only have one aspect ratio. */
			CenterWindow			= utility::make_flag<1>, /* Center the window to the screen when created. */
			SavePosition			= utility::make_flag<2>, /* Saves the window position when closed. */
			SaveSize				= utility::make_flag<3>, /* Saves the window size when closed. */

			Default					= RestrictAspectRatio | SavePosition | SaveSize,
		};


		//! @brief struct defining the properties of a given Window
		struct Properties
		{
			std::wstring title = L"Window Title";
			Style style = Style::Default;
			Flags flags = Flags::Default;
			vec2i position;
			vec2i size = { 1280, 720 }; // size always refers to client size
			int monitor_idx = 0; // which monitor to use. (0 is always the primary)
			SaveID save = NoSaveID;
		};

	public:

		FISSION_API static std::unique_ptr<Window> Create( const Properties & props, Graphics * pGraphics, IEventHandler * event_handler );

		virtual ~Window() = default;
	public:

	//	virtual Input * GetInput() = 0;

		virtual void SetEventHandler( IEventHandler * handler ) = 0;

		virtual void SetTitle( const std::wstring & title ) = 0;

		virtual std::wstring GetTitle() = 0;

		virtual void SetStyle( Style style ) = 0;

		virtual Style GetStyle() = 0;

		virtual void SetSize( const vec2i & size ) = 0;

		virtual Resource::SwapChain * GetSwapChain() = 0;

		virtual vec2i GetSize() = 0;

		//! @brief Destroy the platform window and exit it's event loop.
		//! @note This function evokes the `IEventHandler::OnClose` function
		//!			before destroying the window.
		virtual void Close() = 0;

		//! @note For entering fullscreen mode, THIS is the monitor that will be used.
		virtual MonitorPtr GetMonitor() = 0;

		//! @brief Set which monitor the window will prefer for fullscreen mode
		virtual void SetMonitor( MonitorPtr ) = 0;

		virtual void DisplayMessageBox( const std::wstring & title, const std::wstring & text ) = 0;
		
		//! experimental: this function might seem pointless, but this is necessary 
		//! for calling platform functions within the same thread of a window
		virtual void Call( std::function<void()> function ) = 0;

		//! @brief Get a Handle to the native window
		virtual native_handle_type native_handle() = 0;

	}; // class Fission::Window

	//! @brief Settings for the Main Window.
	//! @warning Used by Fission's internal API, do not use for save id.
	static constexpr Window::SaveID MainWindowID = 0; 

	//! @brief Start of valid Window ID's
	static constexpr Window::SaveID WindowSaveID_Begin = 1; 

} // namespace Fission

_lazer_Define_Flag_Enum_Operators( Fission::Window::Flags );
