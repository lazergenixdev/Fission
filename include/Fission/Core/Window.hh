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
#include <Fission/config.h>
#include <Fission/Base/Size.h>
#include <Fission/Base/String.h>
#include <Fission/Base/Math/Vector.h>
#include <Fission/Base/Utility/BitFlag.h>
#include <Fission/Core/Input/Event.hh>
#include <Fission/Core/Monitor.hh>
#include <Fission/Core/Graphics.hh>

// todo: implement fixed aspect-ratio flag
// todo: implement fullscreen

namespace Fission
{
	struct IFWindow : public IFObject
	{
		/*! @brief Native window handle type */
		using native_handle_type = Platform::WindowHandle;

		using SaveID = int;

		static constexpr SaveID NoSaveID = -1; /*!< Window properties will not be saved. */


		enum class Style
		{
			Borderless,     /*!< Window has no border or title bar */
			Border,         /*!< Window has a border around it with a close button */
			BorderSizeable, /*!< Window how a border with min/max and able to be resized */
			Fullscreen,     /*!< Window is fullscreen */

			Default = Border
		};

		enum Flags : utility::bit_flag<32>
		{
			None                = 0,
			RestrictAspectRatio = utility::make_flag<0,32>, /*!< Restrict the window to only have one aspect ratio. */
			CenterWindow        = utility::make_flag<1,32>, /*!< Center the window to the screen when created. */
			SavePosition        = utility::make_flag<2,32>, /*!< Saves the window position when closed. */
			SaveSize            = utility::make_flag<3,32>, /*!< Saves the window size when closed. */
			AlwaysOnTop         = utility::make_flag<4,32>,

			Default	= RestrictAspectRatio | SavePosition | SaveSize | CenterWindow,
		};
		using Flag_t = utility::bit_flag_t<Flags>;


		//! @brief struct defining the properties of a given Window
		struct Properties
		{
			string         title       = "Window Title"; /*!< Title of the window that will show in the title bar. */
			base::size     size        = { 1280, 720 };  /*!< Client size of the window. */
			base::vector2i position    = {};             /*!< Position of the window's Top-Left coordinate. */
			Style          style       = Style::Default; /*!< Style of the window, can be one of @IFWindow::Style. */
			Flag_t         flags       = Flags::Default; /*!< Flags of the window, can be any combination of @IFWindow::Flags. */
			int            monitor_idx = 0;              /*!< Monitor to use for fullscreen mode. (0 is always the primary) */
			SaveID         save        = NoSaveID;       /*!< Save ID for this window, used for saving window properties for next startup. */
		};

		struct CreateInfo
		{
			Properties        wProperties;
			IFEventHandler *  pEventHandler;
		};

	public:

	//	virtual Input * GetInput() = 0;

	//	virtual void SetEventHandler( IEventHandler * _Event_Handler ) = 0;

	//	virtual void SetTitle( const string & _Title ) = 0;

	//	virtual string GetTitle() = 0;

	//	virtual void SetStyle( Style _Style ) = 0;

	//	virtual Style GetStyle() = 0;

	//	virtual void SetSize( const base::size & _Size ) = 0;

	//	virtual base::size GetSize() = 0;

		virtual Resource::IFSwapChain * GetSwapChain() = 0;

		//! @brief Destroy the platform window and exit it's event loop.
		//! @note This function evokes the `IEventHandler::OnClose` function
		//!			before destroying the window.
		virtual void Close() = 0;

		//! @note For entering fullscreen mode, THIS is the monitor that will be used.
	//	virtual MonitorPtr GetMonitor() = 0;

		//! @brief Set which monitor the window will prefer for fullscreen mode
	//	virtual void SetMonitor( MonitorPtr ) = 0;

		//! @brief Get a Handle to the native window
		virtual native_handle_type native_handle() = 0;

	}; // struct Fission::IFWindow

	//! @brief Settings for the Main Window.
	//! @warning Used by Fission's internal API, do not use for save id.
	static constexpr IFWindow::SaveID MainWindowID = 0; 

	//! @brief Start of valid Window ID's
	static constexpr IFWindow::SaveID WindowSaveID_Begin = 1; 

} // namespace Fission

_Fission_Declare_Enum_Operators( Fission::IFWindow::Flags );
