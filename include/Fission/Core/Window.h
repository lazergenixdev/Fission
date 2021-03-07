#pragma once
#include "Fission/config.h"
#include "Input/Event.h"
#include "lazer/utility.h"

namespace Fission
{

	class Window
	{
	public:

		/*!< Native window handle type */
		using native_handle_type = Platform::WindowHandle;


		enum class Style
		{
			Borderless,			// Window has no border
			Border,				// Window has a border around it with a close button
			BorderSizeable,		// Window how a border with min/max and able to be resized

			Default = Border
		};

		enum class Flags : utility::flag
		{
			None					= 0,
			Fullscreen				= utility::make_flag<0>, /* Window is in fullscreen mode. */
			RestrictAspectRatio		= utility::make_flag<1>, /* Restrict the window to only have one AspectRatio. */
		//	CenterWindow			= utility::make_flag<2>, /* Center the window to the screen when created. */
			SavePosition			= utility::make_flag<3>, /* Saves the window position when closed. */
			SaveSize				= utility::make_flag<4>, /* Saves the window size when closed. */
			IsMainWindow			= utility::make_flag<5>, /* The window is the Main Window. */

			Default					= RestrictAspectRatio | SavePosition,
		};


		/// <summary>
		/// struct defining the properties of a given Window
		/// </summary>
		struct Properties
		{
			std::wstring title = L"Window Title";
			Style style = Style::Default;
			Flags flags = Flags::Default;
			std::optional<vec2i> position;
			vec2i size = { 1280, 720 }; // size always refers to client size
		};

	public:

		/// <summary>
		/// Window::Create Summary.
		/// </summary>
		FISSION_API static std::unique_ptr<Window> Create( const Properties & props, IEventHandler * event_handler );

		virtual ~Window() = default;
	public:

	//	virtual Input * GetInput() = 0;

		virtual void SetEventHandler( IEventHandler * handler ) = 0;

		virtual void SetTitle( const std::wstring & title ) = 0;

		virtual std::wstring GetTitle() = 0;

		virtual void SetStyle( Style style ) = 0;

		virtual Style GetStyle() = 0;

		virtual void SetSize( const vec2i & size ) = 0;

		virtual vec2i GetSize() = 0;

		virtual void Close() = 0;

		virtual void DisplayMessageBox( const std::wstring & title, const std::wstring & text ) = 0;
		
		// experimental, this function might seem pointless, 
		// but this is necessary for calling platform functions within the same thread of a window
		virtual void Call( std::function<void()> function ) = 0;

		//! @brief Get a Handle to the native window
		virtual native_handle_type native_handle() = 0;

	};

}

_lazer_Define_Flag_Enum_Operators( Fission::Window::Flags );
