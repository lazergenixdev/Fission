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
#include <Fission/Core/Graphics.hh>
#include <Fission/Core/Monitor.hh>
#include <Fission/Core/Input/Event.hh>
#include <Fission/Base/Size.hpp>
#include <Fission/Base/String.hpp>
#include <Fission/Base/Math/Vector.hpp>
#include <Fission/Base/util/BitFlag.hpp>

// todo: implement fixed aspect-ratio flag
// todo: implement fullscreen

namespace Fission
{
	namespace wnd
	{
		enum class Style
		{
			Borderless,     /*!< Window has no border or title bar */
			Border,         /*!< Window has a border around it with a close button */
			BorderSizeable, /*!< Window how a border with min/max and able to be resized */
			Fullscreen,     /*!< Window is fullscreen */

			Default = Border
		};

		enum Flags : util::bit_flag<32>
		{
			None                = 0,
			RestrictAspectRatio = util::make_flag<0>, /*!< Restrict the window to only have one aspect ratio. */
			CenterWindow        = util::make_flag<1>, /*!< Center the window to the screen when created. */
			SavePosition        = util::make_flag<2>, /*!< Saves the window position when closed. */
			SaveSize            = util::make_flag<3>, /*!< Saves the window size when closed. */
			AlwaysOnTop         = util::make_flag<4>,

			Default	= RestrictAspectRatio | SavePosition | SaveSize | CenterWindow,
		};
		using Flag_t = util::bit_flag_t<Flags>;

		//! @brief struct defining the properties of a given Window
		struct Properties
		{
			string  title       = "Window Title";       /*!< Title of the window that will show in the title bar. */
			size2   size        = { 1280, 720 };        /*!< Client size of the window. */
			v2i32   position    = {};                   /*!< Position of the window's Top-Left coordinate. */
			Style   style       = Style::Default;       /*!< Style of the window, can be one of @IFWindow::Style. */
			Flag_t  flags       = Flags::Default;       /*!< Flags of the window, can be any combination of @IFWindow::Flags. */
			int     monitor_idx = MonitorIdx_Automatic; /*!< Monitor to use for fullscreen mode. (0 is always the primary) */
		};

	} // namespace wnd

	struct Window : public ManagedObject
	{
		/*! @brief Native window handle type */
		using native_handle_type = Platform::WindowHandle;

		struct CreateInfo
		{
			wnd::Properties  properties;
			EventHandler *   pEventHandler;
		};

	public:

	//	virtual Input * GetInput() = 0;

	//	virtual void SetEventHandler( IEventHandler * _Event_Handler ) = 0;

		// debug
		virtual void _debug_set_position( v2i32 p ) = 0;

		virtual void SetTitle( const string & _Title ) = 0;

		virtual string GetTitle() = 0;

		virtual void SetStyle( wnd::Style _Style ) = 0;

		virtual wnd::Style GetStyle() = 0;

		virtual void SetSize( const size2 & _Size ) = 0;

		virtual size2 GetSize() = 0;

		virtual gfx::SwapChain * GetSwapChain() = 0;

		//! @brief Destroy the platform window and exit it's event loop.
		//! @note This function evokes the `IEventHandler::OnClose` function
		//!			before destroying the window.
		virtual void Close() = 0;

		//! @note For entering fullscreen mode, THIS is the monitor that will be used.
		virtual MonitorPtr GetMonitor() = 0;

		//! @brief Set which monitor the window will prefer for fullscreen mode
	//	virtual void SetMonitor( MonitorPtr ) = 0;

		//! @brief Get a Handle to the native window
		virtual native_handle_type native_handle() = 0;

	}; // struct Fission::Window

} // namespace Fission

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