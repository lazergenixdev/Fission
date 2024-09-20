#include <internal.hpp>
#include "common.hpp"
#include <Fission/core/engine.hpp>
#include <Fission/core/input/Keys.hpp>
#include <Fission/platform/utils.hpp>

using namespace fs;

struct Window_Class {
    static constexpr wchar_t const* name = L"Fission-Engine";
};

struct Window_Style {
    DWORD value, ex;
};

namespace fs {
    struct Window_Proxy {
        static LRESULT _message_callback(HWND, UINT, WPARAM, LPARAM) noexcept;
    };
}

auto Window::create (Window_Create_Info const& info) -> bool
{
    log::debug(PLATFORM_"Creating window...");

    HINSTANCE instance = GetModuleHandleW(nullptr);

    WNDCLASSEXW window_class_info = {
        .cbSize = sizeof(WNDCLASSEXW),
        .lpfnWndProc = Window::_setup_callback,
        .hInstance = instance,
        .hIcon = LoadIcon(NULL, IDI_SHIELD),
        .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
        .lpszClassName = Window_Class::name,
    };

    if (!RegisterClassExW(&window_class_info)) {
        report_error("RegisterClassExW", PLATFORM_"Failed to register window class");
        return 1;
    }
    else log::verbose(PLATFORM_"Registered window class");

    Window_Style style {
        WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION,
        0
    };

    auto title = win32_to_wide_string(info.title);

    _handle = CreateWindowExW(
        style.ex,            // Ex Style
        Window_Class::name,  // Window Class Name
        title,               // Window Title
        style.value,         // Style
        CW_USEDEFAULT,       // Position X
        CW_USEDEFAULT,       // Position Y
        CW_USEDEFAULT,       // Width
        CW_USEDEFAULT,       // Height
        NULL, NULL,          // Parent Window, Menu
        instance,            // Instance
        this                 // UserData
    );

    LocalFree(title);

    return 0;
}

void Window::show() { ShowWindow(_handle, SW_SHOW); }

void Window::close()
{
    // "A thread cannot use DestroyWindow to destroy
    //  a window created by a different thread."
    if (_handle) SendMessageW(_handle, WM_USER_DESTROY, 0, 0);
}

auto Window::supported_modes() -> u32
{
    return Windowed | Windowed_Resizeable | Windowed_Fullscreen | Exclusive_Fullscreen;
}

Window::~Window ()
{
    log::verbose(PLATFORM_"Destroying Window...");

    if (_handle) {
        log::verbose(PLATFORM_"Sending 'WM_DESTROY'...");
        // Send WM_DESTROY...
        DestroyWindow(_handle);
        // Destroy window...
        MSG msg;
        while (GetMessageW(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        _handle = NULL;
    }

    if (!UnregisterClassW(Window_Class::name, GetModuleHandleW(nullptr))) {
        report_error("UnregisterClassW", PLATFORM_"Failed to unregister window class");
	}
    else log::verbose(PLATFORM_"Unregistered window class");
}

LRESULT Window::process_message(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (Msg) {
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK: {
        u32 id = keys::Unknown;
        if (Msg == WM_LBUTTONDOWN || Msg == WM_LBUTTONDBLCLK) { id = keys::Mouse_Left; }
        if (Msg == WM_RBUTTONDOWN || Msg == WM_RBUTTONDBLCLK) { id = keys::Mouse_Right; }
        if (Msg == WM_MBUTTONDOWN || Msg == WM_MBUTTONDBLCLK) { id = keys::Mouse_Middle; }
        //if( Msg == WM_XBUTTONDOWN || Msg == WM_XBUTTONDBLCLK ) { button = ( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 ) ? 3 : 4; }
        SetCapture(hWnd);
        Event event {
            .type = Event_Key_Down,
        //    .timestamp = last_timestamp.QuadPart,
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
        if (Msg == WM_LBUTTONUP) { id = keys::Mouse_Left; }
        if (Msg == WM_RBUTTONUP) { id = keys::Mouse_Right; }
        if (Msg == WM_MBUTTONUP) { id = keys::Mouse_Middle; }
        //	if( Msg == WM_XBUTTONUP ) { button = ( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 ) ? 3 : 4; }
        ReleaseCapture();
        Event event {
            .type = Event_Key_Up,
        //    .timestamp = last_timestamp.QuadPart,
            .key_up = {
                .key_id = id,
            }
        };
        event_queue.append(event);
        return 0;
    }

    case WM_MOUSEMOVE: {
        mouse_position = (v2s32)reinterpret_cast<v2s16&>(lParam);
        event_queue.append({
            .type = Event_Mouse_Move_Absolute,
        //    .timestamp = last_timestamp.QuadPart,
            .mouse_move_absolute = {
                mouse_position,
            }
        });
        break;
    }

    default: break;
    }
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

LRESULT Window_Proxy::_message_callback(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (Msg)
    {
    case WM_USER_DESTROY: {
        log::verbose(PLATFORM_"Got message to destroy window");
        engine.flags &=~ Engine::Running;
        DestroyWindow(hWnd);
        return 0;
    }

    case WM_CLOSE: {
        log::verbose(PLATFORM_"Got message requesting to close window");
        engine.flags &=~ Engine::Running;
        break;
    }

    case WM_DESTROY: {
        engine.flags &=~ Engine::Running;
        PostQuitMessage(0);
        return 0;
    }

    default: break;
    }

    auto p_window = reinterpret_cast<fs::Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    return p_window->process_message(hWnd, Msg, wParam, lParam);
}

LRESULT platform::Window::_setup_callback(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) noexcept
{
    if (Msg == WM_NCCREATE) {
        auto p_create_struct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto p_window = reinterpret_cast<Window*>(p_create_struct->lpCreateParams);

        if (p_window == nullptr) {
            log::error(PLATFORM_"Failed to receive window [WM_NCCREATE]");
            goto Default_Procedure;
        }

        log::verbose(PLATFORM_"Setting up message callback");

        SetWindowLongPtrW(hWnd, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(p_window));
        SetWindowLongPtrW(hWnd, GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(Window_Proxy::_message_callback));
    }
Default_Procedure:
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}
