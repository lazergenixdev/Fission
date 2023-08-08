#include <Fission/Core/Window.hh>
#include <Fission/Base/Assert.hpp>
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Input/Keys.hh>
#include "../Common.h"
using namespace fs;

void EnableDarkModeAPI() noexcept;
void SetDarkModeForWindow(HWND hwnd) noexcept;
void display_win32_fatal_error(WCHAR const* what) noexcept;

extern fs::Engine engine;

__FISSION_BEGIN__

#define WindowClassName L"Fission Window Class"

inline LARGE_INTEGER last_timestamp;

static LRESULT WinProcSetup(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT Window_Message_Callback(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    QueryPerformanceCounter(&last_timestamp);
    switch (msg)
    {
    case WM_CLOSE: {
        if (engine.flags & engine.fWindow_Destroy_Enable) {
            DestroyWindow(hwnd);
            engine.running = false;
        }
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }

    case WM_CREATE: {
        SetDarkModeForWindow(hwnd);
        break;
    }

    // No application menu pls thaaaaaaaaaaaaaaaaaaaks
    case WM_COMMAND: return 0;
    case WM_SYSCOMMAND:
    {
        if (wp == SC_CLOSE) break;
        switch (wp & 0xFFF0)
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

    case WM_SIZE: {
        if (wp == SIZE_MINIMIZED) {
            engine.minimized = true;
        }
        else if (engine.minimized) {
            std::unique_lock lock(engine._mutex);
            engine.minimized = false;
            engine._event.notify_one();
        }
        return 0;
    }

    case WM_SETCURSOR: {
        if ((short)lp == HTCLIENT) {
            SetCursor(LoadCursorW(NULL, IDC_ARROW));
            return TRUE;
        }
        break;
    }

    default:break;
    }

    auto wnd = (Window*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    return wnd->_win32_ProcessMessage(hwnd, msg, wp, lp);
}

LRESULT Window::_win32_ProcessMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static constexpr u64 modifier_map[3] = {
        fs::keys::Mod_Shift,
        fs::keys::Mod_Control,
        fs::keys::Mod_Alt,
    };

    switch (msg)
    {
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN: {
        Event event{
            .timestamp = (unsigned)last_timestamp.QuadPart,
            .type = Event_Key_Down,
            .key_down = {
                .key_id = (u32)wp,
            }
        };
        if (!(lp & 0x40000000)) { // not a repeat
            event_queue.append(event);

            if (wp >= VK_SHIFT && wp <= VK_MENU)
                engine.modifier_keys |= modifier_map[wp - VK_SHIFT];
        }
        break;
    }

    case WM_SYSKEYUP:
    case WM_KEYUP: {
        Event event{
            .timestamp = (unsigned)last_timestamp.QuadPart,
            .type = Event_Key_Up,
            .key_up = {
                .key_id = (u32)wp,
            }
        };
        event_queue.append(event);
        if (wp >= VK_SHIFT && wp <= VK_MENU)
            engine.modifier_keys &= ~modifier_map[wp - VK_SHIFT];
        break;
    }

    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK: {
        u32 id = keys::Unknown;
        if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { id = keys::Mouse_Left; }
        if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { id = keys::Mouse_Right; }
        if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { id = keys::Mouse_Middle; }
        //if( Msg == WM_XBUTTONDOWN || Msg == WM_XBUTTONDBLCLK ) { button = ( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 ) ? 3 : 4; }
        SetCapture(hwnd);
        Event event{
            .timestamp = (unsigned)last_timestamp.QuadPart,
            .type = Event_Key_Down,
            .key_down = {
                .key_id = id,
            }
        };
        event_queue.append(event);
        return 0;
    }

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP: {
        u32 id = keys::Unknown;
        if (msg == WM_LBUTTONUP) { id = keys::Mouse_Left; }
        if (msg == WM_RBUTTONUP) { id = keys::Mouse_Right; }
        if (msg == WM_MBUTTONUP) { id = keys::Mouse_Middle; }
        //	if( Msg == WM_XBUTTONUP ) { button = ( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 ) ? 3 : 4; }
        ReleaseCapture();
        Event event{
            .timestamp = (unsigned)last_timestamp.QuadPart,
            .type = Event_Key_Up,
            .key_up = {
                .key_id = id,
            }
        };
        event_queue.append(event);
        return 0;
    }
    case WM_MOUSEWHEEL: {
        Event event{
            .timestamp = (unsigned)last_timestamp.QuadPart,
        };
        _mouse_wheel_delta += GET_WHEEL_DELTA_WPARAM(wp);

        {
            std::scoped_lock lock{event_queue.access_mutex};
            event.key_down.key_id = keys::Mouse_WheelUp;
            while (_mouse_wheel_delta >= WHEEL_DELTA) {
                event.type = Event_Key_Down;
                event_queue.array.emplace_back(event);
                event.type = Event_Key_Up;
                event_queue.array.emplace_back(event);
                _mouse_wheel_delta -= WHEEL_DELTA;
            }
            event.key_down.key_id = keys::Mouse_WheelDown;
            while (_mouse_wheel_delta <= WHEEL_DELTA) {
                event.type = Event_Key_Down;
                event_queue.array.emplace_back(event);
                event.type = Event_Key_Up;
                event_queue.array.emplace_back(event);
                _mouse_wheel_delta += WHEEL_DELTA;
            }
        }
        break;
    }

    case WM_MOUSEMOVE: {
        mouse_position = (v2s32)reinterpret_cast<v2s16&>(lp);
        break;
    }

    case WM_KILLFOCUS: {
        Event event{
            .timestamp = (unsigned)last_timestamp.QuadPart,
            .type = Event_Focus_Lost,
        };
        event_queue.append(event);
        engine.modifier_keys = 0;
        break;
    }

    case WM_CHAR: {
        if (_codepoint_builder.append(wp & 0xFFFF)) {
            Event event{
                .timestamp = (unsigned)last_timestamp.QuadPart,
                .type      = Event_Character_Input,
                .character_input = {
                    .codepoint = _codepoint_builder.codepoint,
                }
            };
            event_queue.append(event);
        }
        break;
    }

    default:break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

#undef assert // I DECIDE WHAT ASSERT DOES, NOT YOU!! BITCH ASS CASSERT
#define assert(R, WHAT) if(!(R)) { display_win32_fatal_error(L##WHAT); return 1; } (void)0

struct Window_Thread_Info {
    Window* window;
    string initial_title;
};

static int window_main(Window_Thread_Info* info) {
    auto&& [window, initial_title] = *info;
    auto hInstance = GetModuleHandleW(nullptr);

    { // Create Window
        std::scoped_lock lock(window->_mutex);

        // wtf
        struct defer {
            Window* wnd;
            ~defer() { wnd->_cv.notify_one(); }
        } _notify{window};

        std::wstring title;

        EnableDarkModeAPI();

        title.reserve(initial_title.count);
        for (u64 i = 0; i < initial_title.count; ++i)
            title.push_back((wchar_t)initial_title.data[i]);

        auto style_from_mode = [](Window_Mode m) -> DWORD {
            switch (m)
            {
            case fs::Windowed:             return(WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_VISIBLE);
            case fs::Windowed_Fullscreen:  return(WS_POPUP | WS_VISIBLE);
            case fs::Exclusive_Fullscreen: return(WS_VISIBLE);
            default:                       return(WS_VISIBLE);
            }
        };
        auto ex_style_from_mode = [](Window_Mode m) {
            return 0;
        };

        RECT wr = { 0, 0, window->width, window->height };
        if(window->mode == Windowed)::AdjustWindowRect(&wr, style_from_mode(window->mode), FALSE);

        window->_handle = CreateWindowExW(
            ex_style_from_mode(window->mode),   // Ex Style
            WindowClassName,                    // Window Class Name
            title.c_str(),                      // Window Title
            style_from_mode(window->mode),      // Style
            CW_USEDEFAULT, CW_USEDEFAULT,       // Position
            wr.right - wr.left,                 // Width
            wr.bottom - wr.top,                 // Height
            NULL, NULL,                         // Parent Window, Menu
            hInstance,                          // Instance
            window                              // UserData
        );

        assert(window->_handle != NULL, "Failed to create window [CreateWindowExW]");
    }

    window->event_queue.array.reserve(100);

    MSG msg = {};
    BOOL bRet;

    while ((bRet = GetMessageW(&msg, nullptr, 0, 0)) != 0)
    {
        assert(bRet != -1, "Failed to get message [GetMessageW]");

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    window->_handle = NULL;

    // CHECK: This may need to be syncronized across threads
    engine.running = false;

    return 0;
}
#undef assert

void Window::set_title(string const& title)
{
    std::wstring ws;

    // TODO: fix stupid conversion
    ws.reserve(title.count);
#if 1
    for (int i = 0; i < title.count; ++i)
        ws.push_back((wchar_t)title.data[i]);
#else
    string_utf16 dst;
    dst.data  = (c16*)ws.data();
    dst.count = title.count;
    convert_utf8_to_utf16(&dst, title);
    ws.resize(dst.count);
#endif

    SetWindowTextW(_handle, ws.c_str());
}

void Window::close() {
    DestroyWindow(_handle);
}

void Window::create(Window_Create_Info* info)
{
    WNDCLASSEXW wClassDesc = {};
    wClassDesc.cbSize = sizeof(WNDCLASSEXW);
    wClassDesc.lpszClassName = WindowClassName;
    wClassDesc.hInstance = GetModuleHandleW(nullptr);
    wClassDesc.lpfnWndProc = WinProcSetup;
    wClassDesc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wClassDesc.hIcon = LoadIcon(NULL, IDI_SHIELD);

    RegisterClassExW(&wClassDesc);

    width  = info->width;
    height = info->height;
    mode   = info->mode;

    static Window_Thread_Info thread_info; // this is stupid
    thread_info.window = this;
    thread_info.initial_title = info->title;

    std::unique_lock lock(_mutex);
    _thread = std::thread(window_main, &thread_info);

    // Wait for window to be created on message thread
    // (HWND will not be valid until window is created)
    _cv.wait(lock);
}

bool Window::is_minimized()
{
    WINDOWPLACEMENT placement;
    placement.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(_handle, &placement);
    return placement.showCmd == SW_SHOWMINIMIZED;
}

bool Window::exists() const {
    return _handle != NULL;
}

Window::~Window() {
    if (_handle) {
        SendMessageW(_handle, WM_DESTROY, 0, 0);
    }
    _thread.join();
    UnregisterClassW(WindowClassName, GetModuleHandleW(nullptr));
}

// Chili Tomato Noodle
static LRESULT WinProcSetup(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    // use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
    if (Msg == WM_NCCREATE)
    {
        // extract ptr to window class from creation data
        const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        Window* const pWnd = reinterpret_cast<Window*>(pCreate->lpCreateParams);
        // sanity check
        if (pWnd == nullptr) display_win32_fatal_error(L"Failed to receive lpCreateParams [WM_NCCREATE]");
        // set WinAPI-managed user data to store ptr to window class
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        // set message proc to normal (non-setup) handler now that setup is finished
        SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Window_Message_Callback));
    }
    // if we get a message before the WM_NCCREATE message, handle with default handler
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

__FISSION_END__