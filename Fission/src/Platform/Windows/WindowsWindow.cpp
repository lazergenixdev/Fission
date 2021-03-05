#include "WindowsWindow.h"
#include "Fission/Core/Console.h"
#include <Fission/Core/Configuration.h>
#include <Xinput.h>

// application defined window messages
#define FISSION_WINEVENT_MSGBOX       ( WM_USER + 0 )
#define FISSION_WINEVENT_CLOSE        ( WM_USER + 1 )
#define FISSION_WINEVENT_SETTITLE     ( WM_USER + 2 )
#define FISSION_WINEVENT_SETSTYLE     ( WM_USER + 3 )
#define FISSION_WINEVENT_SETSIZE      ( WM_USER + 4 )

// TODO: Restrict Window Aspect Ratio
// TODO: Fix Sizing of window when switching Window styles
// TODO: Use raw input for keyboard and mouse

namespace Fission::Platform {

    static bool SetFullscreen( HWND hwnd, WindowsWindow * pwnd, bool fullscreen )
    {
        if( fullscreen ) 
        {
            DEVMODE fullscreenSettings;
            bool isChangeSuccessful;
           // RECT windowBoundary;

            vec2i res = { 1280, 720 };

            EnumDisplaySettingsW( NULL, 0, &fullscreenSettings );
            fullscreenSettings.dmPelsWidth = res.x;
            fullscreenSettings.dmPelsHeight = res.y;
            fullscreenSettings.dmBitsPerPel = 32;
            fullscreenSettings.dmDisplayFrequency = 60;
            fullscreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

            SetWindowPos( hwnd, HWND_TOP, 0, 0, res.x, res.y, SWP_SHOWWINDOW );
            isChangeSuccessful = ChangeDisplaySettings( &fullscreenSettings, CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL;
            SetWindowLongPtr( hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST );
            SetWindowLongPtr( hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE );
            ShowWindow( hwnd, SW_SHOW );

            return ( isChangeSuccessful );
        }
        else
        {
            bool isChangeSuccessful;

            SetWindowLongPtr( hwnd, GWL_EXSTYLE, WS_EX_LEFT );
            SetWindowLongPtr( hwnd, GWL_STYLE, WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_VISIBLE );
            isChangeSuccessful = ChangeDisplaySettings( NULL, CDS_RESET ) == DISP_CHANGE_SUCCESSFUL;
            auto size = pwnd->GetWindowsSize();
            auto pos = pwnd->GetPosition();
            SetWindowPos( hwnd, HWND_NOTOPMOST, pos.x, pos.y, size.x, size.y, SWP_SHOWWINDOW );
            ShowWindow( hwnd, SW_RESTORE );

            return ( isChangeSuccessful );
        }
    }

    WindowsWindow::WindowsWindow( const Properties & props, IEventHandler * event_handler )
        : m_Properties( props ), pEventHandler( event_handler )
    {
        std::unique_lock lock( m_AccessMutex );
    //    m_bRestrictAR = ( props.aspectRatio == vec2i{} );
        m_WindowThread = std::thread( Run, this );
        // Wait for window to be created on separate thread
        m_AccessCV.wait( lock );
    }

    WindowsWindow::~WindowsWindow()
    {
        if( m_Handle ) Close();
        m_WindowThread.join();
    }

    void WindowsWindow::SetEventHandler( IEventHandler * handler )
    {
        pEventHandler = handler;
    }

    void WindowsWindow::SetTitle( const std::wstring & title )
    {
        m_Properties.title = title;
        SendMessageW( m_Handle, FISSION_WINEVENT_SETTITLE, (WPARAM)m_Properties.title.c_str(), 0 );
    }

    std::wstring WindowsWindow::GetTitle()
    {
        return m_Properties.title;
    }

    void WindowsWindow::SetStyle( Style style )
    {
        m_Properties.style = style;
        auto nstyle = GetWindowsStyle();
        SendMessageW( m_Handle, FISSION_WINEVENT_SETSTYLE, (WPARAM)&nstyle, 0 );
    }

    Window::Style WindowsWindow::GetStyle()
    {
        return m_Properties.style;
    }

    void WindowsWindow::SetSize( const vec2i & size )
    {
        m_Properties.size = size;
        auto wsize = GetWindowsSize();
        SendMessageW( m_Handle, FISSION_WINEVENT_SETSIZE, (WPARAM)wsize.x, (LPARAM)wsize.y );
    }

    vec2i WindowsWindow::GetSize()
    {
        RECT cr;
        GetClientRect( m_Handle, &cr );
        m_Properties.size = { ( cr.right - cr.left ), ( cr.bottom - cr.top ) };
        return m_Properties.size;
    }

    void WindowsWindow::DisplayMessageBox( const std::wstring & title, const std::wstring & info )
    {
        SendMessageW( m_Handle, FISSION_WINEVENT_MSGBOX, (WPARAM)title.c_str(), (LPARAM)info.c_str() );
    }

    Window::native_handle_type WindowsWindow::native_handle()
    {
        return m_Handle;
    }

    void WindowsWindow::Close()
    {
        SendMessageW( m_Handle, FISSION_WINEVENT_CLOSE, 0, 0 );
    }

    void WindowsWindow::ProcessMessage( UINT Msg, WPARAM wParam, LPARAM lParam )
    {
    }

    void WindowsWindow::Run( WindowsWindow * pthis )
    {
        int ec = 0;
        try 
        {
            { // Create Window

                std::lock_guard lock( pthis->m_AccessMutex );

                MONITORINFO info = {};
                info.cbSize = sizeof MONITORINFO;
                GetMonitorInfoW( MonitorFromPoint( { -1, -1 }, MONITOR_DEFAULTTOPRIMARY ), &info );

                auto size = pthis->GetWindowsSize();

                if( !pthis->m_Properties.position )
                    pthis->m_Properties.position = vec2i{ info.rcMonitor.right - size.x, info.rcMonitor.bottom - size.y } / 2;

                vec2i & pos = pthis->m_Properties.position.value();

                pos.x = std::max( pos.x, 0 );
                pos.y = std::max( pos.y, 0 );

                pthis->m_Handle = CreateWindowExW(
                    0L,
                    WindowClass::GetName(),
                    pthis->m_Properties.title.c_str(),
                    pthis->GetWindowsStyle(),
                    pos.x, pos.y,
                    size.x, size.y,
                    NULL, NULL,
                    WindowClass::GetInstance(),
                    nullptr
                );

                SetWindowLongPtrW( pthis->m_Handle, GWLP_USERDATA, (LONG_PTR)pthis );

                auto sStyle = L"";
                switch( pthis->m_Properties.style )
                {
                case Style::Border: sStyle = L"Border"; break;
                case Style::Borderless: sStyle = L"Borderless"; break;
                case Style::BorderSizeable: sStyle = L"BorderSizeable"; break;
                default:break;
                }

                Console::WriteLine( Colors::LightBlue, L"Created Window with properties:" );
                Console::WriteLine( Colors::LightBlue, L" - Position: (%d, %d)", pos.x, pos.y );

                Console::WriteLine( Colors::LightBlue, L" - Size: [%d x %d]", pthis->m_Properties.size.x, pthis->m_Properties.size.y );
                Console::WriteLine( Colors::LightBlue, L" - Style: %s", sStyle );

            }

            // Tell Constructor to exit, window should be created now
            pthis->m_AccessCV.notify_one();

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

            ec = (int)msg.wParam;
        }
        catch( ... ) {
            MessageBoxA( NULL, "Summary: Don't Fucking throw exceptions from the event handler!!", "Exception Caught in Message Loop", MB_OK );
            ec = 0x45;
        }

        CloseEventArgs closeEvent{ ec };
        pthis->pEventHandler->OnClose( closeEvent );

        DestroyWindow( pthis->m_Handle );
        pthis->m_Handle = NULL;

        if( bool( pthis->m_Properties.flags & Flags::IsMainWindow ) )
        {
            Configuration::SetWindowConfig( pthis->m_Properties );
        }
    }

    void WindowsWindow::PollGamePad( WindowsWindow * pthis )
    {
    }

    LRESULT WindowsWindow::BaseWindowsProc( _In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam )
    {
        auto pWindow = reinterpret_cast<WindowsWindow *>( GetWindowLongPtrW( hWnd, GWLP_USERDATA ) );

        Event native_event = { hWnd, Msg, wParam, lParam };
        //Console::WriteLine( L"HWND[%i] -- MSG:%i", Colors::White, hWnd, Msg );

        switch( Msg )
        {

        /************* Input Messages *************/
        case WM_MOUSEMOVE:
        {
            MouseMoveEventArgs ev{ &native_event };
            ev.position.x = short( lParam );
            ev.position.y = short( lParam >> 16 );
            pWindow->pEventHandler->OnMouseMove( ev );
            pWindow->m_MouseTracker.OnMouseMove( hWnd );
            break;
        }
        case WM_MOUSELEAVE:
        {
            MouseLeaveEventArgs ev{ &native_event };
            pWindow->pEventHandler->OnMouseLeave( ev );
            pWindow->m_MouseTracker.Reset( hWnd );
            break;
        }
        case WM_ACTIVATE:
        {
            if( wParam == WA_INACTIVE && bool( pWindow->m_Properties.flags & Flags::Fullscreen ) )
                ShowWindow( hWnd, SW_MINIMIZE );
            break;
        }
        case WM_KEYDOWN:
        {
        //    Console::WriteLine( L"[WindowId:%x] KEYDOWN: wParam=%i lParam=%i", Colors::Gray, (int)hWnd, wParam, lParam );
            // No Repeats
            if( lParam & 0x40000000 )
                break;

            KeyDownEventArgs ev{ &native_event };
            ev.native_event = &native_event;
            ev.key = key_from_win32( (int)wParam );

            if( ev.key == Keys::F11 )
                if( bool( pWindow->m_Properties.flags & Flags::Fullscreen ) )
                {
                    if( SetFullscreen( hWnd, pWindow, false ) )
                        utility::remove_flag<(size_t)Flags::Fullscreen>( pWindow->m_Properties.flags );
                }
                else
                {
                    if( SetFullscreen( hWnd, pWindow, true ) )
                        utility::set_flag<(size_t)Flags::Fullscreen>( pWindow->m_Properties.flags );
                }

            pWindow->pEventHandler->OnKeyDown( ev );
            break;
        }
        case WM_KEYUP:
        {
            KeyUpEventArgs ev{ &native_event };
            ev.key = key_from_win32( (int)wParam );
            pWindow->pEventHandler->OnKeyUp( ev );
            break;
        }
        case WM_CHAR:
        {
            TextInputEventArgs ev{ &native_event };
            ev.character = (wchar_t)wParam;
            pWindow->pEventHandler->OnTextInput( ev );
            break;
        }
        case WM_LBUTTONDOWN:
        {
            KeyDownEventArgs ev{ &native_event };
            ev.key = Keys::Mouse_Left;
            pWindow->pEventHandler->OnKeyDown( ev );
            SetCapture( hWnd );
            break;
        }
        case WM_LBUTTONUP:
        {
            KeyUpEventArgs ev{ &native_event };
            ev.key = Keys::Mouse_Left;
            pWindow->pEventHandler->OnKeyUp( ev );
            ReleaseCapture();
            break;
        }
        case WM_RBUTTONDOWN:
        {
            KeyDownEventArgs ev{ &native_event };
            ev.key = Keys::Mouse_Right;
            pWindow->pEventHandler->OnKeyDown( ev );
            break;
        }
        case WM_RBUTTONUP:
        {
            KeyUpEventArgs ev{ &native_event };
            ev.key = Keys::Mouse_Right;
            pWindow->pEventHandler->OnKeyUp( ev );
            break;
        }
        case WM_MOUSEWHEEL:
        {
            KeyDownEventArgs downEvent{ &native_event };
            KeyUpEventArgs upEvent{ &native_event };
            pWindow->m_MouseWheelDelta += GET_WHEEL_DELTA_WPARAM( wParam );
            downEvent.key = upEvent.key = Keys::Mouse_WheelUp;
            while( pWindow->m_MouseWheelDelta >= WHEEL_DELTA ) {
                pWindow->pEventHandler->OnKeyDown( downEvent );
                pWindow->pEventHandler->OnKeyUp( upEvent );
                pWindow->m_MouseWheelDelta -= WHEEL_DELTA;
            }
            downEvent.key = upEvent.key = Keys::Mouse_WheelDown;
            while( pWindow->m_MouseWheelDelta <= WHEEL_DELTA ) {
                pWindow->pEventHandler->OnKeyDown( downEvent );
                pWindow->pEventHandler->OnKeyUp( upEvent );
                pWindow->m_MouseWheelDelta += WHEEL_DELTA;
            }
            break;
        }


        /************* User Messages *************/
        case FISSION_WINEVENT_SETTITLE:
        {
            SetWindowTextW( hWnd, (const wchar_t *)wParam );
            return 0;
        }
        case FISSION_WINEVENT_SETSIZE:
        {
            SetWindowPos( hWnd, NULL, 0, 0, (int)wParam, (int)lParam, SWP_NOMOVE );
            return 0;
        }
        case FISSION_WINEVENT_SETSTYLE:
        {
            SetWindowLongPtrW( hWnd, GWL_STYLE, pWindow->GetWindowsStyle() );

            auto size = pWindow->GetWindowsSize();
            SetWindowPos( hWnd, NULL, 0, 0, size.x, size.y, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW );

            return 0;
        }
        case FISSION_WINEVENT_MSGBOX:
        {
            const wchar_t * title = reinterpret_cast<const wchar_t *>( wParam );
            const wchar_t * info = reinterpret_cast<const wchar_t *>( lParam );
            MessageBoxW( hWnd, info, title, MB_OK );
            return 0;
        }
        case FISSION_WINEVENT_CLOSE:
        {
            PostQuitMessage( 0 );
            return 0;
        }


        /************* System Messages *************/
        case WM_SIZE:
        {
            if( pWindow )
            {
                if( wParam == SIZE_MINIMIZED )
                    pWindow->pEventHandler->OnHide();
                else
                    pWindow->pEventHandler->OnShow();
            }
            break;
        }
        case WM_WINDOWPOSCHANGED:
        {
            //auto pWindowPos = reinterpret_cast<WINDOWPOS *>( lParam );

            //Console::WriteLine( L"[windowposchanged] NOMOVE=%i NOOWNERZORDER=%i,", Colors::White, 
            //    bool( pWindowPos->flags & SWP_NOMOVE ),
            //    bool( pWindowPos->flags & SWP_NOOWNERZORDER )
            //);

            //if( pWindowPos->hwndInsertAfter == HWND_TOP && pWindow->m_pGraphics )
            //    ; //pWindow->m_pGraphics->SetFullscreen( pWindow->m_pGraphics->IsFullscreen() );

            break;
        }
        case WM_MOVE:
        {
            auto point = MAKEPOINTS( lParam );
            if( pWindow )
                pWindow->m_Properties.position = vec2i::from( point );
            break;
        }
        //case WM_SIZING:
        //{
        //    // TODO: add AR restriction
        //    RECT * pRect = reinterpret_cast<RECT *>( lParam );
        //    int width = pRect->right - pRect->left, 
        //        height = pRect->bottom - pRect->top;
        //    const int min_width = 300, 
        //        min_height = pWindow->m_bRestrictAR ? 
        //            ( min_width * pWindow->m_Properties.aspectRatio.y ) / pWindow->m_Properties.aspectRatio.x : 210; // hmm
        //    switch( wParam )
        //    {
        //    case WMSZ_TOP:
        //    {
        //        if( height < min_height ) { pRect->top = pRect->bottom - min_height; }
        //        break;
        //    }
        //    case WMSZ_LEFT:
        //    {
        //        if( width < min_width ) { pRect->left = pRect->right - min_width; }
        //        break;
        //    }
        //    case WMSZ_RIGHT:
        //    {
        //        if( width < min_width ) { pRect->right = pRect->left + min_width; }
        //        break;
        //    }
        //    case WMSZ_BOTTOM:
        //    {
        //        if( height < min_height ) { pRect->bottom = pRect->top + min_height; }
        //        break;
        //    }
        //    case WMSZ_TOPLEFT:
        //    {
        //        if( height < min_height ) { pRect->top = pRect->bottom - min_height; }
        //        if( width < min_width ) { pRect->left = pRect->right - min_width; }
        //        break;
        //    }
        //    case WMSZ_TOPRIGHT:
        //    {
        //        if( height < min_height ) { pRect->top = pRect->bottom - min_height; }
        //        if( width < min_width ) { pRect->right = pRect->left + min_width; }
        //        break;
        //    }
        //    case WMSZ_BOTTOMLEFT:
        //    {
        //        if( height < min_height ) { pRect->bottom = pRect->top + min_height; }
        //        if( width < min_width ) { pRect->left = pRect->right - min_width; }
        //        break;
        //    }
        //    case WMSZ_BOTTOMRIGHT:
        //    {
        //        if( height < min_height ) { pRect->bottom = pRect->top + min_height; }
        //        if( width < min_width ) { pRect->right = pRect->left + min_width; }
        //        break;
        //    }
        //    default:break;
        //    }
        //    break;
        //}
        case WM_SETCURSOR:
        {
            if( (short)lParam == HTCLIENT )
            {
                SetCursorEventArgs ev{ &native_event };
                ev.cursor = pWindow->m_Cursor;
                pWindow->pEventHandler->OnSetCursor( ev );
                ev.cursor->Use();
                //pWindow->m_Cursor = ev.cursor;

                //HCURSOR hCurs3;             // cursor handle 
                //BYTE ANDmaskCursor[] =
                //{
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //    0xFF, 0xFF,
                //};
                //BYTE XORmaskCursor[] =
                //{
                //    0xFF, 0xFF,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0x80, 0x01,
                //    0xFF, 0xFF,
                //};
                //hCurs3 = CreateCursor( WindowClass::GetInstance(),   // app. instance 
                //    8,                 // horizontal position of hot spot 
                //    8,                 // vertical position of hot spot 
                //    16,                // cursor width 
                //    16,                // cursor height 
                //    ANDmaskCursor,     // AND mask 
                //    XORmaskCursor );   // XOR mask 
                //SetCursor( hCurs3 );
                return TRUE;
            }
            break;
        }
        case WM_COMMAND:
        case WM_SYSCOMMAND:
        {
            //Console::WriteLine( 
            //    L"\"%s\" [WindowId:%x] wParam=%i lParam=%i", 
            //    Colors::Gray, 
            //    pWindow->m_Properties.title.c_str(), 
            //    (int)hWnd, 
            //    wParam, 
            //    lParam 
            //);
            break;
        }
        case WM_DESTROY:
            if( pWindow )
            PostQuitMessage( 0 );
           break;

        default:break;
        }

        return DefWindowProc( hWnd, Msg, wParam, lParam );
    }

    DWORD WindowsWindow::GetWindowsStyle()
    {
        switch( m_Properties.style )
        {
        case Window::Style::Borderless:
            return WS_POPUP | WS_VISIBLE;
        case Window::Style::Border:
            return WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_VISIBLE;
        case Window::Style::BorderSizeable:
            return WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;

        default:return 0;
        }
    }

    vec2i WindowsWindow::GetWindowsSize()
    {
        switch( m_Properties.style )
        {
        case Style::Borderless:
            return m_Properties.size;
        default:break;
        }

        RECT wr = {
            0, 0,
            m_Properties.size.x,
            m_Properties.size.y,
        };

        ::AdjustWindowRect( &wr, WS_SYSMENU | WS_THICKFRAME | WS_BORDER | WS_CAPTION, FALSE );

        return vec2i( wr.right - wr.left, wr.bottom - wr.top );
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Window Class ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    HINSTANCE WindowsWindow::WindowClass::GetInstance()
    {
        return Get().m_hInstance;
    }

    const wchar_t * WindowsWindow::WindowClass::GetName()
    {
        return Get().m_Name;
    }

    WindowsWindow::WindowClass::WindowClass()
        : m_Name( L"wc_Fission" ), m_hInstance( GetModuleHandle( nullptr ) )
    {
        WNDCLASSW wc = {};
        wc.lpszClassName = m_Name;
        wc.hInstance = m_hInstance;
        wc.lpfnWndProc = WindowsWindow::BaseWindowsProc;
        wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );

        RegisterClassW( &wc );
    }

    WindowsWindow::WindowClass::~WindowClass()
    {
        UnregisterClassW( m_Name, m_hInstance );
    }

    WindowsWindow::WindowClass & WindowsWindow::WindowClass::Get()
    {
        static WindowClass wc;
        return wc;
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
