#include "../Common.h"
#include <Fission/Base/Assert.hpp>
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Console.hh>
#include <Fission/Core/Input/Keys.hh>
#include <Dbt.h>
using namespace fs;

void EnableDarkModeAPI() noexcept;
void SetDarkModeForWindow(HWND hwnd) noexcept;
void display_win32_fatal_error(WCHAR const* what) noexcept;


#define __hide_cursor() while (::ShowCursor(FALSE) >= 0)
#define __show_cursor() while (::ShowCursor(TRUE)   < 0)

extern fs::Engine engine;

__FISSION_BEGIN__

#define WindowClassName L"Fission Window Class"
#define HID_USAGE_PAGE_GENERIC  0x01
#define HID_USAGE_GENERIC_MOUSE 0x02

#define WM_USER_SHOW_CURSOR (WM_USER + 0)
#define WM_USER_HIDE_CURSOR (WM_USER + 1)

inline LARGE_INTEGER last_timestamp;

LRESULT Window::Message_Callback(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    QueryPerformanceCounter(&last_timestamp);
    switch (msg)
    {
    case WM_CLOSE: {
        if (engine.flags & engine.fWindow_Destroy_Enable) {
            DestroyWindow(hwnd);
            engine.flags &=~ engine.fRunning;
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

    case WM_SETCURSOR: {
        // Probably can improve this with some user-defined stuff..
        if ((short)lp == HTCLIENT) {
            SetCursor(LoadCursorW(NULL, IDC_ARROW));
            return TRUE;
        }
        break;
    }

    case WM_DEVICECHANGE: {
        // "A device has been added to or removed from the system."
        if (wp == DBT_DEVNODES_CHANGED) {
        //  enumerate_displays(engine.displays);
        }
        break;
    }

    case WM_USER_SHOW_CURSOR: {
        __show_cursor();
        return 0;
    }

    case WM_USER_HIDE_CURSOR: {
        __hide_cursor();
        return 0;
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
            .timestamp = last_timestamp.QuadPart,
            .type = Event_Key_Down,
            .key_down = {
                .key_id = (u32)wp,
            }
        };
        if (!(lp & 0x40000000)) { // not a repeat
            event_queue.append(event);

        // TODO: There is a better way:
        // https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#keystroke-message-flags
            if (wp >= VK_SHIFT && wp <= VK_MENU)
                engine.modifier_keys |= modifier_map[wp - VK_SHIFT];
        }
        break;
    }

    case WM_ACTIVATE: {
        if (!(_flags & platform::Window_Enable_Mouse_Deltas)) break;

        if (wp & WA_ACTIVE) {
            RECT rect;
            GetClientRect(_handle, &rect);
            MapWindowPoints(_handle, nullptr, reinterpret_cast<POINT*>(&rect), 2);
            ClipCursor(&rect);
            __hide_cursor();
        } else {
            ClipCursor(nullptr);
            __show_cursor();
        }
        break;
    }

    case WM_INPUT:
    {
        if (!(_flags & platform::Window_Enable_Mouse_Deltas)) break;
        
        static std::vector<RAWINPUT> rawBuffer;
        
        UINT result, size;
        // first get the size of the input data
        result = GetRawInputData(reinterpret_cast<HRAWINPUT>(lp), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
        if (result == -1) break;
        
        rawBuffer.resize(size/sizeof(RAWINPUT));
        
        result = GetRawInputData(reinterpret_cast<HRAWINPUT>(lp),RID_INPUT,rawBuffer.data(),&size,sizeof(RAWINPUTHEADER));
        if (result != size) break;

        // process the raw input data
        for (auto const& raw_input : rawBuffer) {
            auto ri = reinterpret_cast<RAWINPUT const&>(raw_input);

            if (ri.header.dwType == RIM_TYPEMOUSE &&
               (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
            {
                Event event{
                    .timestamp = last_timestamp.QuadPart,
                    .type = Event_Mouse_Move_Relative,
                    .mouse_move_relative = {
                        fs::v2s32(ri.data.mouse.lLastX, ri.data.mouse.lLastY),
                    }
                };
                event_queue.append(event);
            }
        }
        break;
    }

    case WM_SYSKEYUP:
    case WM_KEYUP: {
        Event event{
            .timestamp = last_timestamp.QuadPart,
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
            .timestamp = last_timestamp.QuadPart,
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
            .timestamp = last_timestamp.QuadPart,
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
            .timestamp = last_timestamp.QuadPart,
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
            while (_mouse_wheel_delta <= -WHEEL_DELTA) {
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
        Event event{
            .timestamp = last_timestamp.QuadPart,
            .type = Event_Mouse_Move_Absolute,
            .mouse_move_absolute = {
                mouse_position,
            }
        };
        event_queue.append(event);
        break;
    }

    case WM_MOVE: {
        if (_flags & platform::Window_Disable_Position_Update); else {
            auto point = MAKEPOINTS(lp);
            position = v2s32::from(point);
        }
        break;
    }

    case WM_SIZE: {
        if (wp == SIZE_MINIMIZED) {
            _flags |= platform::Window_Minimized;
        }
        else if (_flags & platform::Window_Minimized) {
            std::unique_lock lock(_mutex);
            _flags &=~ platform::Window_Minimized;
            _cv.notify_one();
        }
        return 0;
    }

    case WM_KILLFOCUS: {
        Event event{
            .timestamp = last_timestamp.QuadPart,
            .type = Event_Focus_Lost,
        };
        event_queue.append(event);
        engine.modifier_keys = 0;
        break;
    }

    case WM_CHAR: {
        if (_codepoint_builder.append(wp & 0xFFFF)) {
            Event event{
                .timestamp = last_timestamp.QuadPart,
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

#undef assert // I DECIDE WHAT ASSERT DOES, NOT YOU!! BITCH ASS CASSERT <- angry boi
#define assert(R, WHAT) if(!(R)) { display_win32_fatal_error(L##WHAT); return 1; } (void)0

HMONITOR get_monitor(int index, HWND hwnd) {
    if (index == Display_Index_Automatic)
        return MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    else if (index < engine.displays.size() && index >= 0)
        return engine.displays[index]._handle;

    else
        return MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
}

struct Window_Thread_Info {
    Window* window;
    string initial_title;
};

int window_main(Window_Thread_Info* info) {
    auto [window, initial_title] = *info;
    auto hInstance = GetModuleHandleW(nullptr);

    { // Create Window
        std::scoped_lock lock(window->_mutex);

        // wtf
        struct defer {
            Window* wnd;
            ~defer() { wnd->_cv.notify_one(); }
        } _notify{window};

        EnableDarkModeAPI();
        
        auto monitor = get_monitor(window->display_index, window->_handle);
        assert(monitor != NULL, "Failed to get monitor handle");

        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(monitor, &mi);

        auto style = window->_get_style();

        RECT wr = { 0, 0, window->width, window->height };
        if (window->mode == Windowed) {
            auto bRet = ::AdjustWindowRectEx(&wr, style.value, FALSE, style.ex);
            assert(bRet != 0, "Failed to calculate window size [AdjustWindowRectEx]");

            auto monitor_size = v2s32(mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top);
            auto offset = (monitor_size - v2s32(wr.right - wr.left, wr.bottom - wr.top)) / 2;
            wr.left   += offset.x;
            wr.right  += offset.x;
            wr.top    += offset.y;
            wr.bottom += offset.y;
        }
        else if (window->mode == Windowed_Fullscreen) {
            wr = mi.rcMonitor;
            window->_flags |= platform::Window_Disable_Position_Update;
        }

        c16 buffer[512]; // Don't think Windows can even handle titles this long...

        string src_title = initial_title;
        src_title.count = std::min(src_title.count, std::size(buffer) - 1);

        string_utf16 win32_title = {.count = std::size(buffer) - 1, .data = buffer};
        convert_utf8_to_utf16(&win32_title, src_title);

        win32_title.data[win32_title.count] = 0; // null terminate

        window->_handle = CreateWindowExW(
            style.ex,                           // Ex Style
            WindowClassName,                    // Window Class Name
            (wchar_t*)win32_title.data,         // Window Title
            style.value,                        // Style
            wr.left, wr.top,                    // Position
            wr.right - wr.left,                 // Width
            wr.bottom - wr.top,                 // Height
            NULL, NULL,                         // Parent Window, Menu
            hInstance,                          // Instance
            window                              // UserData
        );

        assert(window->_handle != NULL, "Failed to create window [CreateWindowExW]");
    }

    {
        RAWINPUTDEVICE rid = {
            .usUsagePage = HID_USAGE_PAGE_GENERIC,
            .usUsage     = HID_USAGE_GENERIC_MOUSE,
            .dwFlags     = 0,
            .hwndTarget  = nullptr,
        };
        auto r = RegisterRawInputDevices(&rid, 1, sizeof(rid));
        assert(r != FALSE, "Failed to register input devices [RegisterRawInputDevices]");
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

    // Window handle is no longer valid
    window->_handle = NULL;

    // This may need to be syncronized across threads, but i'm too fucking lazy
    engine.flags &=~ engine.fRunning;

    return 0;
}
#undef assert

v2s32 Window::_get_size() {
    RECT wr = {0, 0, width, height};
    auto style = _get_style();
    AdjustWindowRectEx(&wr, style.value, NULL, style.ex);
    return {wr.right - wr.left, wr.bottom - wr.top};
}

Window::_Style Window::_get_style() {
    switch (mode)
    {
    case Windowed:             return {0, WS_VISIBLE|WS_MINIMIZEBOX|WS_SYSMENU|WS_CAPTION};
    case Windowed_Fullscreen:  return {WS_EX_APPWINDOW, WS_VISIBLE|WS_POPUP};
    case Exclusive_Fullscreen: return {0, WS_VISIBLE};
    default:                   return {};
    }
}

void Window::set_mode(Window_Mode mode) {

    HMONITOR display_handle = get_monitor(display_index, _handle);
    if (display_handle == NULL) {
        display_win32_fatal_error(L"Failed to get monitor handle");
        return;
    }

    this->mode = mode;
    auto style = _get_style();
    SetWindowLongW(_handle, GWL_STYLE  , style.value);
    SetWindowLongW(_handle, GWL_EXSTYLE, style.ex);

    rs32 r;
    {
        MONITORINFO info;
        info.cbSize = sizeof(MONITORINFO);
        GetMonitorInfoW(display_handle, &info);
        r = rs32::from_win32(info.rcMonitor);
    }

    switch (mode)
    {
    default: // windowed right?
    case fs::Windowed: {
        auto size = _get_size();
        SetWindowPos(_handle, NULL, position.x, position.y, size.x, size.y, SWP_FRAMECHANGED);
        _flags &=~ platform::Window_Disable_Position_Update;
        break;
    }
    case fs::Windowed_Fullscreen:
    case fs::Exclusive_Fullscreen: // same as other fullscreen for now
        _flags |= platform::Window_Disable_Position_Update;
        SetWindowPos(_handle, NULL, r.x.low, r.y.low, r.x.distance(), r.y.distance(), SWP_ASYNCWINDOWPOS|SWP_FRAMECHANGED);
        break;
    }
}

bool Window::is_using_mouse_deltas() {
    return _flags & platform::Window_Enable_Mouse_Deltas;
}

void Window::set_using_mouse_deltas(bool use) {
    if (use) {
        _flags |=  platform::Window_Enable_Mouse_Deltas;

        RECT rect;
        GetClientRect(_handle, &rect);
        MapWindowPoints(_handle, nullptr, reinterpret_cast<POINT*>(&rect), 2);
        ClipCursor(&rect);
        SendMessageW(_handle, WM_USER_HIDE_CURSOR, 0, 0);
    }
    else {
        _flags &=~ platform::Window_Enable_Mouse_Deltas;

        ClipCursor(nullptr);
        SendMessageW(_handle, WM_USER_SHOW_CURSOR, 0, 0);
    }
}

void Window::set_title(string const& title)
{
    c16 buffer[512]; // Don't think Windows can even handle titles this long...
    
    string input = title;
    // Here we cut off the input if it is too long, but this could be a problem for multi-byte codepoints...
    if (input.count > std::size(buffer) - 1) input.count = std::size(buffer) - 1;

    string_utf16 utf16 = {.count = std::size(buffer) - 1, .data = buffer};
    convert_utf8_to_utf16(&utf16, input);

    utf16.data[utf16.count] = 0; // null terminate
    SetWindowTextW(_handle, (wchar_t*)utf16.data);
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
    wClassDesc.lpfnWndProc = Window::Callback_Setup;
    wClassDesc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wClassDesc.hIcon = LoadIcon(NULL, IDI_SHIELD);

    RegisterClassExW(&wClassDesc);

    width  = info->width;
    height = info->height;
    mode   = info->mode;

    Window_Thread_Info thread_info; // this is stupid
    thread_info.window = this;
    thread_info.initial_title = info->title;

    _flags |= platform::Window_Init_Completed;

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
    if (_flags& platform::Window_Init_Completed) {
        _thread.join();
        UnregisterClassW(WindowClassName, GetModuleHandleW(nullptr));
    }
}

void Window::sleep_until_not_minimized() {
    std::unique_lock lock(_mutex);
    _cv.wait(lock, [this]() { return !(_flags & platform::Window_Minimized); });
}

// Chili Tomato Noodle
LRESULT Window::Callback_Setup(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
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
        SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Window::Message_Callback));
    }
    // if we get a message before the WM_NCCREATE message, handle with default handler
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

__FISSION_END__
