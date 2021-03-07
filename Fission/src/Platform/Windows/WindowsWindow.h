#pragma once
#include "Fission/Core/Window.h"
#include <mutex>
#include <condition_variable>
#include <thread>

namespace Fission::Platform
{
	struct WindowStyleWin32 {
		DWORD Style, ExStyle;
	};

	class WindowsWindow : public Window
	{
	public:

		WindowsWindow( const Properties & props, IEventHandler * event_handler );
		~WindowsWindow();

		virtual void SetEventHandler( IEventHandler * handler ) override;

		virtual void SetTitle( const std::wstring & title ) override;

		virtual std::wstring GetTitle() override;

		virtual void SetStyle( Style style ) override;

		virtual Style GetStyle() override;

		virtual void SetSize( const vec2i & size ) override;

		virtual vec2i GetSize() override;

		virtual void DisplayMessageBox( const std::wstring & title, const std::wstring & info ) override;

		virtual void Call( std::function<void()> function ) override;

		virtual native_handle_type native_handle() override;

		virtual void Close() override;

		vec2i GetPosition() const {
			return m_Properties.position.value();
		}

	private:
		class WindowClass
		{
		public:
			static HINSTANCE GetInstance();
			static const wchar_t * GetName();
		private:
			WindowClass();
			~WindowClass();

			const wchar_t * m_Name;
			HINSTANCE m_hInstance;

			static WindowClass & Get();
		};

		class MouseTrackEvents
		{
			bool m_bMouseTracking = false;
		public:
			void OnMouseMove( HWND hwnd );
			void Reset( HWND hwnd );
		};

	private:

		void ProcessMessage( UINT Msg, WPARAM wParam, LPARAM lParam );

		static void Run( WindowsWindow * pthis );

		// Todo: implement gamepad support when bored
		static void PollGamePad( WindowsWindow * pthis );

		static LRESULT CALLBACK BaseWindowsProc(
			_In_ HWND hWnd,
			_In_ UINT Msg,
			_In_ WPARAM wParam,
			_In_ LPARAM lParam
		);

	public:
		DWORD GetWindowsStyle();
		vec2i GetWindowsSize();

	private:

		short m_MouseWheelDelta = 0;

		Cursor * m_Cursor = Cursor::Get( Cursor::Default_Arrow );

		Properties m_Properties;
		bool m_bRestrictAR = false;

		WindowHandle m_Handle = NULL;

		IEventHandler * pEventHandler = IEventHandler::Default();
		MouseTrackEvents m_MouseTracker;

		std::mutex m_AccessMutex;
		std::condition_variable m_AccessCV;

		std::thread m_WindowThread;
		std::thread m_GamePadPollingThread;

	};

}

