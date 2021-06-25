#pragma once
#include <Fission/Base/Utility/SmartPointer.h>
#include "WinWindowManager.h"

namespace Fission::Platform
{
	struct WindowStyleWin32 {
		DWORD Style, ExStyle;
	};

	extern LRESULT CALLBACK SetupWindowsProc( _In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam );
	extern LRESULT CALLBACK MainWindowsProc( _In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam );

	class WindowsWindow : public IFWindow
	{
	public:

		WindowsWindow( const CreateInfo * info, GlobalWindowInfo * gInfo );

	//	virtual void SetEventHandler( IEventHandler * handler ) override;

		virtual void SetTitle( const string & title ) override;

		virtual string GetTitle() override;

		virtual void SetStyle( Style style ) override;

		virtual Style GetStyle() override;

		virtual void SetSize( const base::size & size ) override;

		virtual base::size GetSize() override;

	//	virtual void Call( std::function<void()> function ) override;

		virtual native_handle_type native_handle() override;

		virtual Resource::IFSwapChain * GetSwapChain() override;

		virtual MonitorPtr GetMonitor() override;

	//	virtual void SetMonitor( MonitorPtr ) override;

		virtual void Close() override;

		virtual void Destroy() override;

		base::vector2i GetPosition() const { return m_Properties.position; }

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
		base::size GetWindowsSize();

	private:
		FPointer<Resource::IFSwapChain> m_pSwapChain;
		IFEventHandler * pEventHandler = IFEventHandler::Default();

		short m_MouseWheelDelta = 0;

		Cursor * m_Cursor = Cursor::Get( Cursor::Default_Arrow );

		Properties m_Properties;
		bool m_bRestrictAR = false;
		bool m_bFullscreenMode = false;

		WindowHandle m_Handle = NULL;

		MouseTrackEvents m_MouseTracker;

		Monitor * m_pMonitor;

		std::mutex m_AccessMutex;
		std::condition_variable m_AccessCV;

		std::thread m_WindowThread;
		std::thread m_GamePadPollingThread;

		GlobalWindowInfo * m_pGlobalInfo;
		bool m_bResetMonitor = false;
	};

}

