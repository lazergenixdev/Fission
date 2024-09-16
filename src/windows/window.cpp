#include <internal.hpp>
#include "common.hpp"
#include <Fission/core/engine.hpp>
#include <Fission/platform/utils.hpp>

using namespace fs;

struct Window_Class {
    static constexpr wchar_t const* name = L"wc_Fission";
};

struct Window_Style {
    DWORD value, ex;
};

auto Window::create (Window_Create_Info const& info) -> bool
{
    log::debug("Creating window...");

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
        report_error("RegisterClassExW", "Failed to register window class");
        return 1;
    }
    else log::verbose("(Win32) Registered window class");

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

void Window::show() {
    ShowWindow(_handle, SW_SHOW);
}

void Window::close()
{
    // "A thread cannot use DestroyWindow to destroy
    //  a window created by a different thread."
    if (_handle) SendMessageW(_handle, WM_USER_DESTROY, 0, 0);
}

Window::~Window () {
    log::verbose("(Win32) Window destructor");

    if (_handle) {
        log::verbose("(Win32) Destroying window...");
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
        report_error("UnregisterClassW", "Failed to unregister window class");
	}
    else log::verbose("(Win32) Unregistered window class");
}

LRESULT Window::handle_message(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) noexcept
{
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

LRESULT platform::Window::_message_callback(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (Msg)
    {
    case WM_USER_DESTROY: {
        log::verbose("(Win32) Got message to destroy window");
        DestroyWindow(hWnd);
        return 0;
    }

    case WM_CLOSE: {
        log::verbose("(Win32) Got message requesting to close window");
        break;
    }

    case WM_DESTROY: {
        engine.flags &=~ engine.fRunning;
        PostQuitMessage(0);
        return 0;
    }

    default: break;
    }

    auto p_window = reinterpret_cast<fs::Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    return p_window->handle_message(hWnd, Msg, wParam, lParam);
}

LRESULT platform::Window::_setup_callback(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) noexcept
{
    if (Msg == WM_NCCREATE) {
        auto p_create_struct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto p_window = reinterpret_cast<Window*>(p_create_struct->lpCreateParams);

        if (p_window == nullptr) {
            log::error("(Win32) Failed to receive window [WM_NCCREATE]");
            goto Default_Procedure;
        }

        log::verbose("(Win32) Setting up message callback");

        SetWindowLongPtrW(hWnd, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(p_window));
        SetWindowLongPtrW(hWnd, GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(Window::_message_callback));
    }
Default_Procedure:
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}
