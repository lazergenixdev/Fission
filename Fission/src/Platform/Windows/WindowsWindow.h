#pragma once
#include <Fission/Base/util/SmartPointer.hpp>
#include "WinWindowManager.h"
#include <condition_variable>

namespace Fission::Platform
{
	struct WindowStyleWin32 {
		DWORD Style, ExStyle;
	};

	class CharacterBuilder
	{
	public:
		CharacterBuilder();

		// @return `True` if codepoint is now complete, `False` otherwise
		bool AppendChar16( char16_t value );

		char32_t GetCodepoint();

	private:
		char32_t temp;
		int bytes_occupied = 0;
	};

	extern LRESULT CALLBACK WindowsProcSetup( _In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam );
	extern LRESULT CALLBACK WindowsProcMain( _In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam );

	class WindowsWindow : public Window
	{
	public:

		WindowsWindow( const CreateInfo * info, GlobalWindowInfo * gInfo );

	//	virtual void SetEventHandler( IEventHandler * handler ) override;


		virtual void _debug_set_position( v2i32 ) override;

		virtual void SetTitle( const string & title ) override;

		virtual string GetTitle() override;

		virtual void SetStyle( wnd::Style style ) override;

		virtual wnd::Style GetStyle() override;

		virtual void SetSize( const size2 & size ) override;

		virtual size2 GetSize() override;

	//	virtual void Call( std::function<void()> function ) override;

		virtual native_handle_type native_handle() override;

		virtual gfx::SwapChain * GetSwapChain() override;

		virtual MonitorPtr GetMonitor() override;

	//	virtual void SetMonitor( MonitorPtr ) override;

		virtual void Close() override;

		virtual void Destroy() override;

		v2i32 GetPosition() const { return m_Properties.position; }

		LRESULT HandleEvent( Event * _Ptr_Event );

	private:

		class MouseTrackEvents
		{
			bool m_bMouseTracking = false;
		public:
			void OnMouseMove( HWND hwnd );
			void Reset( HWND hwnd );
		};

	private:


		void MessageThreadMain();

	public:
		DWORD GetWindowsStyle();
		size2 GetWindowsSize();

	private:
		fsn_ptr<gfx::SwapChain> m_pSwapChain;
		EventHandler * pEventHandler = EventHandler::Default();

		short m_MouseWheelDelta = 0;

		Cursor * m_Cursor = Cursor::Get( Cursor::Default_Arrow );

		wnd::Properties m_Properties;
		bool m_bRestrictAR = false;
		bool m_bFullscreenMode = false;

		WindowHandle m_Handle = NULL;

		MouseTrackEvents m_MouseTracker;
		CharacterBuilder m_CharBuilder;

		Monitor * m_pMonitor;

		std::mutex m_AccessMutex;
		std::condition_variable m_AccessCV;

		std::thread m_WindowThread;
		std::thread m_GamePadPollingThread;

		GlobalWindowInfo * m_pGlobalInfo;
		bool m_bResetMonitor = false;
		
		v2i32 _test_position = {};
	};

}

