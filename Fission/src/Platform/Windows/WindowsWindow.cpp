#include "WindowsWindow.h"
#include <Fission/Base/Exception.h>
#include <Fission/Core/Console.hh>
#include <Fission/Core/Configuration.hh>
#include <Fission/Platform/System.h>
#include <Xinput.h>
#include <Dbt.h>

#include "../../ConfigurationImpl.h"
#include "../Monitor.h"

// application defined window messages
//#define FISSION_WM_MSGBOX       ( WM_USER + 0 )
#define FISSION_WM_CLOSE        ( WM_USER + 1 )
#define FISSION_WM_SETTITLE     ( WM_USER + 2 )
#define FISSION_WM_SETSTYLE     ( WM_USER + 3 )
#define FISSION_WM_SETSIZE      ( WM_USER + 4 )
#define FISSION_WM_CALLEXTERNAL ( WM_USER + 5 )

// TODO: Restrict Window Aspect Ratio
// TODO: Fix Sizing of window when switching Window styles
// TODO: Use raw input for keyboard and mouse

namespace Fission::Platform
{

	LRESULT CALLBACK MainWindowsProc( _In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam )
	{
		// retrieve ptr to window class
		WindowsWindow * const pWnd = reinterpret_cast<WindowsWindow *>( GetWindowLongPtrW( hWnd, GWLP_USERDATA ) );

		Event event = { hWnd, Msg, wParam, lParam };
		// forward message to window class handler
		return pWnd->HandleEvent( &event );
	}

	LRESULT CALLBACK SetupWindowsProc( _In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam )
	{
		// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
		if( Msg == WM_NCCREATE )
		{
			// extract ptr to window class from creation data
			const CREATESTRUCTW * const pCreate = reinterpret_cast<CREATESTRUCTW *>( lParam );
			WindowsWindow * const pWnd = reinterpret_cast<WindowsWindow *>( pCreate->lpCreateParams );
			// sanity check
			FISSION_ASSERT( pWnd != nullptr );
			// set WinAPI-managed user data to store ptr to window class
			SetWindowLongPtrW( hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( pWnd ) );
			// set message proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtrW( hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>( &MainWindowsProc ) );

			Event event = { hWnd, Msg, wParam, lParam };
			// forward message to window class handler
			return pWnd->HandleEvent( &event );
		}
		// if we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProcW( hWnd, Msg, wParam, lParam );
	}

    WindowsWindow::WindowsWindow( const CreateInfo * info, GlobalWindowInfo * gInfo )
        : m_Properties( info->wProperties ), pEventHandler( info->pEventHandler ), m_pGlobalInfo( gInfo )
    {
        std::unique_lock lock( m_AccessMutex );

        m_WindowThread = std::thread( [this] { MessageThreadMain(); } );

        // Wait for window to be created on message thread
        // (HWND will not be valid until window is created)
        m_AccessCV.wait( lock );
    }
//
//    void WindowsWindow::SetEventHandler( IEventHandler * handler )
//    {
//        pEventHandler = handler;
//    }
//
    void WindowsWindow::SetTitle( const string & title )
    {
        m_Properties.title = title;
        auto _New_Title = m_Properties.title.utf16();
        SendMessageW( m_Handle, FISSION_WM_SETTITLE, (WPARAM)_New_Title.c_str(), 0 );
    }

    string WindowsWindow::GetTitle()
    {
        return m_Properties.title;
    }

    void WindowsWindow::SetStyle( Style style )
    {
        SendMessageW( m_Handle, FISSION_WM_SETSTYLE, (WPARAM)&style, 0 );
	}

    IFWindow::Style WindowsWindow::GetStyle()
    {
        return m_Properties.style;
    }

	//void WindowsWindow::Call( std::function<void()> function )
	//{
	//	SendMessageW( m_Handle, FISSION_WM_CALLEXTERNAL, (WPARAM)&function, 0 );
	//}
//
    base::size WindowsWindow::GetSize()
    {
        RECT cr;
        GetClientRect( m_Handle, &cr );
        m_Properties.size = { ( cr.right - cr.left ), ( cr.bottom - cr.top ) };
        return m_Properties.size;
    }

    void WindowsWindow::SetSize( const base::size & size )
    {
        SendMessageW( m_Handle, FISSION_WM_SETSIZE, (WPARAM)size.w, (LPARAM)size.h );
    }

    IFWindow::native_handle_type WindowsWindow::native_handle()
    {
        return m_Handle;
    }

    Resource::IFSwapChain * WindowsWindow::GetSwapChain()
    {
        return m_pSwapChain.get();
    }

    MonitorPtr WindowsWindow::GetMonitor()
    {
        if( m_bResetMonitor )
        {
            m_bResetMonitor = false;
            if( /*m_Properties.style != Style::Fullscreen &&*/ m_Properties.monitor_idx == MonitorIdx_Automatic )
            {
                auto hMonitor = MonitorFromWindow( m_Handle, MONITOR_DEFAULTTONEAREST );
                Monitor * pMonitor = nullptr;
                for( auto && mon : Monitor::GetMonitors() )
                {
                    if( mon->native_handle() == hMonitor )
                    {
                        pMonitor = mon;
                        break;
                    }
                }

                if( pMonitor == nullptr )
                    FISSION_THROW( "Window Error", .append( "Monitor not found! (caused by new monitor being connected)" ) )

                m_pMonitor = pMonitor;
            }
        }
        return m_pMonitor;
    }
//
//    void WindowsWindow::SetMonitor( MonitorPtr a )
//    {
//        m_pMonitor = a;
//    }
//
    void WindowsWindow::Close()
    {
        SendMessageW( m_Handle, FISSION_WM_CLOSE, 0, 0 );
    }

    void WindowsWindow::Destroy()
    {
        if( m_Handle )
            SendMessageW( m_Handle, FISSION_WM_CLOSE, 0, 0 );
        m_WindowThread.join();
        delete this;
    }


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Message Handler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//

	LRESULT WindowsWindow::HandleEvent( Event * e )
	{
        auto && [hWnd, Msg, wParam, lParam] = *e;

        switch( Msg )
        {
        case WM_DESTROY:
        case FISSION_WM_CLOSE:
            PostQuitMessage( 0 );
        break;

        case WM_COMMAND: return 0;
        case WM_SYSCOMMAND:
        {
            if( wParam == SC_CLOSE ) break;
            switch( wParam & 0xFFF0 )
            {
            case SC_KEYMENU:
            case SC_MOUSEMENU:
            case SC_CLOSE:
                return 0;
            default:
                break;
            }
            break;
        }

        case WM_MOVE:
        {
            auto point = MAKEPOINTS( lParam );
            m_Properties.position = base::vector2i::from( point );
            m_bResetMonitor = true;
            break;
        }

        // Determine whether the window is being minimized
        case WM_SIZE:
            if( wParam == SIZE_MINIMIZED )
                pEventHandler->OnHide();
            else
                pEventHandler->OnShow();
        break;

        case WM_MOUSEMOVE:
        {
            MouseMoveEventArgs ev{ e };
            ev.position.x = short( lParam );
            ev.position.y = short( lParam >> 16 );
            pEventHandler->OnMouseMove( ev );
            m_MouseTracker.OnMouseMove( hWnd );
        }
        break;

        case WM_MOUSELEAVE:
        {
            MouseLeaveEventArgs ev{ e };
            pEventHandler->OnMouseLeave( ev );
            m_MouseTracker.Reset( hWnd );
        }
        break;

		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
		{
			Keys::Key button = Keys::Unknown;
			if( Msg == WM_LBUTTONDOWN || Msg == WM_LBUTTONDBLCLK ) { button = Keys::Mouse_Left; }
			if( Msg == WM_RBUTTONDOWN || Msg == WM_RBUTTONDBLCLK ) { button = Keys::Mouse_Right; }
			if( Msg == WM_MBUTTONDOWN || Msg == WM_MBUTTONDBLCLK ) { button = Keys::Mouse_Middle; }
			//if( Msg == WM_XBUTTONDOWN || Msg == WM_XBUTTONDBLCLK ) { button = ( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 ) ? 3 : 4; }
            SetCapture( hWnd );
			KeyDownEventArgs ev{ e };
			ev.key = button;
			pEventHandler->OnKeyDown( ev );
		}
		return 0;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			Keys::Key button = Keys::Unknown;
			if( Msg == WM_LBUTTONUP ) { button = Keys::Mouse_Left; }
			if( Msg == WM_RBUTTONUP ) { button = Keys::Mouse_Right; }
			if( Msg == WM_MBUTTONUP ) { button = Keys::Mouse_Middle; }
		//	if( Msg == WM_XBUTTONUP ) { button = ( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 ) ? 3 : 4; }
			ReleaseCapture();
			KeyUpEventArgs ev{ e };
			ev.key = button;
			pEventHandler->OnKeyUp( ev );
		}
		return 0;

        case WM_CHAR:
        {
            TextInputEventArgs ev{ e };
            ev.codepoint = (char32_t)wParam&0xFFFF; // TODO: this is not always a codepoint, pls fix
            pEventHandler->OnTextInput( ev );
        }
        break;

        case WM_MOUSEWHEEL:
        {
            KeyDownEventArgs downEvent{ e };
            KeyUpEventArgs upEvent{ e };
            m_MouseWheelDelta += GET_WHEEL_DELTA_WPARAM( wParam );

            downEvent.key = upEvent.key = Keys::Mouse_WheelUp;
            while( m_MouseWheelDelta >= WHEEL_DELTA ) {
                pEventHandler->OnKeyDown( downEvent );
                pEventHandler->OnKeyUp( upEvent );
                m_MouseWheelDelta -= WHEEL_DELTA;
            }
            downEvent.key = upEvent.key = Keys::Mouse_WheelDown;
            while( m_MouseWheelDelta <= WHEEL_DELTA ) {
                pEventHandler->OnKeyDown( downEvent );
                pEventHandler->OnKeyUp( upEvent );
                m_MouseWheelDelta += WHEEL_DELTA;
            }
        }
        break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            KeyDownEventArgs ev{ e };
            ev.repeat = ( lParam & 0x40000000 );
            ev.key = key_from_win32( (int)wParam );
            pEventHandler->OnKeyDown( ev );
        }
        break;

        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            KeyUpEventArgs ev{ e };
            ev.key = key_from_win32( (int)wParam );
            pEventHandler->OnKeyUp( ev );
        }
        break;

        case WM_SETCURSOR:
            if( (short)lParam == HTCLIENT )
            {
                SetCursor(LoadCursorW(NULL,IDC_ARROW));
                //SetCursorEventArgs ev{ e };
                //ev.cursor = m_Cursor;
                //pEventHandler->OnSetCursor( ev );
                //if( ev.bUseCursor ) ev.cursor->Use();
                return TRUE;
            }
        break;

        case WM_DEVICECHANGE:
        {
            // TODO: do something here. anything..
            if( wParam == DBT_DEVNODES_CHANGED )
            {
            }
        }
        break;


        case FISSION_WM_SETTITLE:
        {
			SetWindowTextW( hWnd, (LPWSTR)wParam );
			return 0;
        }
        break;

        case FISSION_WM_SETSTYLE:
        {
            auto pStyle = reinterpret_cast<Style *>( wParam );
            auto prevStyle = m_Properties.style;
            if( *pStyle == prevStyle ) return 0;

            m_Properties.style = *pStyle;

            if( prevStyle == Style::Fullscreen )
            {
                auto wstyle = GetWindowsStyle();
                SetWindowLongW( hWnd, GWL_STYLE, wstyle );
                SetWindowLongPtrW( hWnd, GWL_EXSTYLE, WS_EX_LEFT );
                SetWindowLongPtrW( hWnd, GWL_STYLE, wstyle );
                auto size = GetWindowsSize();
                auto pos = GetPosition();
                m_pMonitor->RevertDisplayMode();
                SetWindowPos( hWnd, HWND_NOTOPMOST, pos.x, pos.y, size.w, size.h, SWP_SHOWWINDOW );
                ShowWindow( hWnd, SW_RESTORE );
                return 0;
            }

            {
                auto wstyle = GetWindowsStyle();
                SetWindowLongW( hWnd, GWL_STYLE, wstyle );
            }

            switch( *pStyle )
            {
            case Style::Fullscreen:
            {
                SetWindowLongPtrW( hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST );
                SetWindowLongPtrW( hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE );
            //  SetWindowLongPtr( pWindow->m_Handle, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP );
                auto size = GetWindowsSize();

                DisplayMode mode = *GetMonitor()->GetCurrentDisplayMode();
                mode.resolution = size;
                m_pMonitor->SetDisplayMode( &mode );

                MONITORINFO mi;
                mi.cbSize = sizeof( mi );

                GetMonitorInfoW( m_pMonitor->native_handle(), &mi );

                SetWindowPos( hWnd, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top, mode.resolution.w, mode.resolution.h, SWP_SHOWWINDOW );
            //    ShowWindow( hWnd, SW_MAXIMIZE );
                return 0;
            }

            default:return 0;
            }
        }
        break;

        case FISSION_WM_SETSIZE:
        {
            base::size new_size = { (int)wParam, (int)lParam };
            if( new_size == m_Properties.size ) return 0;

            m_Properties.size = new_size;
            auto size = GetWindowsSize();

            if( m_Properties.style == Style::Fullscreen )
            {
                DisplayMode mode = *m_pMonitor->GetCurrentDisplayMode();
                mode.resolution = new_size;
                m_pMonitor->SetDisplayMode(&mode);
            }

            SetWindowPos( hWnd, NULL, 0, 0, size.w, size.h, SWP_NOMOVE|SWP_NOREPOSITION );

            ResizeEventArgs ev{ e };
            ev.size = new_size;
            pEventHandler->OnResize( ev );
            return 0;
        }
        break;

        default:break;
        }
        return DefWindowProcW( hWnd, Msg, wParam, lParam );
	}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! End Message Handler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//


    void WindowsWindow::MessageThreadMain()
    {
        ExitCode wExitCode = 1;
        CloseEventArgs closeEvent{ wExitCode };

        { // Create Window

            std::lock_guard lock( m_AccessMutex );

            //if( pthis->m_Properties.save != NoSaveID )
            //    ConfigImpl::Get().GetWindowProperties( &pthis->m_Properties );

            auto monitors = Monitor::GetMonitors();
            if( m_Properties.monitor_idx == MonitorIdx_Automatic )
            {
                m_bResetMonitor = true;
                GetMonitor(); // Cause our monitor to be reset.
            }
            else
            if( m_Properties.monitor_idx < monitors.size() )
                m_pMonitor = monitors[m_Properties.monitor_idx];
            else
                m_pMonitor = monitors[0], m_Properties.monitor_idx = 0;

            auto size = GetWindowsSize();

            auto & pos = m_Properties.position;
            pos.x = std::max( pos.x, 0 );
            pos.y = std::max( pos.y, 0 );

            if( m_Properties.flags & Flags::CenterWindow )
            {
                auto mode = m_pMonitor->GetCurrentDisplayMode();
                auto hMonitor = m_pMonitor->native_handle();
                MONITORINFO info; info.cbSize = sizeof( info );
                GetMonitorInfoA( hMonitor, &info );
                auto offset = base::vector2i( mode->resolution.w - size.w, mode->resolution.h - size.h ) / 2;
                pos = base::vector2i(info.rcMonitor.left,info.rcMonitor.top) + offset;
            }

            m_Handle = CreateWindowExW(
                0L,                                         // Ex Style
                m_pGlobalInfo->WindowClassName,             // Window Class Name
                (LPWSTR)m_Properties.title.utf16().c_str(), // Window Title
                GetWindowsStyle(),                          // Style
                pos.x, pos.y,                               // Position
                size.w, size.h,                             // Size
                NULL, NULL,                                 // Parent Window, Menu
                m_pGlobalInfo->hInstance,                   // Instance
                this                                        // UserData
            );

            // We failed to create the window (very unlikely)
            if( m_Handle == NULL )
            {
                char what[100];
                sprintf( what, "'CreateWindowExW()' returned 0. [hInstance=%p]", (void*)m_pGlobalInfo->hInstance );
                System::ShowSimpleMessageBox( "Unable to create window!", what, System::Error );

                goto quick_exit;
            }

            // TODO: CreateSwapChain() is able to throw an exception, handle the exception.
            Resource::IFSwapChain::CreateInfo scInfo = { this };
            m_pSwapChain = m_pGlobalInfo->pGraphics->CreateSwapChain( scInfo );


        // Window is now created successfully, print out some debug info:

            auto sStyle = "<style>";
            switch( m_Properties.style )
            {
            case Style::Border:         sStyle = "Border";         break;
            case Style::Borderless:     sStyle = "Borderless";     break;
            case Style::BorderSizeable: sStyle = "BorderSizeable"; break;
            case Style::Fullscreen:     sStyle = "Fullscreen";     break;
            default:break;
            }

            Console::WriteLine( Colors::LightBlue, "Created Window with properties:" );
            Console::WriteLine( Colors::LightBlue, " - Position: (%d, %d)", pos.x, pos.y );

            Console::WriteLine( Colors::LightBlue, " - Size: [%d x %d]", m_Properties.size.w, m_Properties.size.h );
            Console::WriteLine( Colors::LightBlue, " - Style: %s", sStyle );
        }

        // Tell Constructor to exit, window should be created now
        m_AccessCV.notify_one();

        try 
        {
            MSG msg = {};
            BOOL bRet;

            while( ( bRet = GetMessageW( &msg, nullptr, 0, 0 ) ) != 0 )
            {
                if( bRet == -1 )
                {
                    // error has occurred
                    break;
                }
                else
                {
                    TranslateMessage( &msg );
                    DispatchMessageW( &msg );
                }
            }

            wExitCode = int( msg.wParam );
        }

        catch( base::runtime_error & e )
        {
            char title[64];
            sprintf( title, "Fatal Error (%s)", e.name() );
            System::ShowSimpleMessageBox( title, e.what(), System::Error, this );
            wExitCode = 0x45;
        }

        catch( ... )
        {
            System::ShowSimpleMessageBox( "Fatal Error", "Exception thrown from window message handler, unable to recover :(", System::Error, this );
            wExitCode = 0x46;
        }

        closeEvent.ExitCode = wExitCode;
        pEventHandler->OnClose( closeEvent );

        DestroyWindow( m_Handle );
        m_Handle = NULL;

        return;

    quick_exit:
        m_AccessCV.notify_one();
        pEventHandler->OnClose( closeEvent );

        //if( pthis->m_Properties.save != NoSaveID )
        //    ConfigImpl::Get().SetWindowProperties( pthis->m_Properties );
    }

    DWORD WindowsWindow::GetWindowsStyle()
    {
        switch( m_Properties.style )
        {
        case IFWindow::Style::Borderless:
            return WS_POPUP | WS_VISIBLE;
        case IFWindow::Style::Border:
            return WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_VISIBLE;
        case IFWindow::Style::BorderSizeable:
            return WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;

        default:return 0;
        }
    }

    base::size WindowsWindow::GetWindowsSize()
    {
        switch( m_Properties.style )
        {
        case Style::Borderless:
        case Style::Fullscreen:
            return m_Properties.size;
        default:break;
        }

        RECT wr = {
            0, 0,
            m_Properties.size.w,
            m_Properties.size.h,
        };

        ::AdjustWindowRect( &wr, WS_SYSMENU | WS_THICKFRAME | WS_BORDER | WS_CAPTION, FALSE );

        return { wr.right - wr.left, wr.bottom - wr.top };
    }

    ///////////////// MOUSETRACKING /////////////////

    void WindowsWindow::MouseTrackEvents::OnMouseMove( HWND hwnd )
    {
        if( !m_bMouseTracking )
        {
            // Enable mouse tracking.
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof( tme );
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_LEAVE;
            TrackMouseEvent( &tme );
            m_bMouseTracking = true;
        }
    }

    void WindowsWindow::MouseTrackEvents::Reset( HWND hwnd )
    {
        m_bMouseTracking = false;
    }

}
