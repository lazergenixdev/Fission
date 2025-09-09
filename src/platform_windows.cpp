#include "Fission/core.hpp"
#include "Windows.h"

LRESULT CALLBACK _message_callback(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) noexcept
{
	using namespace fission;
    switch (Msg)
    {
    //case WM_USER_DESTROY: {
    //    log::verbose("Got message to destroy window");
    //    engine.flags &=~ Engine::Running;
    //    DestroyWindow(hWnd);
    //    return 0;
    //}

    case WM_CLOSE: {
        log::verbose("Got message requesting to close window");
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

	return DefWindowProcW(hWnd, Msg, wParam, lParam);
    //auto p_window = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    //return p_window->process_message(hWnd, Msg, wParam, lParam);
}

BEGIN_NAMESPACE(os)

auto init() -> fission::Result {
	bool attached = false, allocated = false;
	if (AttachConsole(ATTACH_PARENT_PROCESS) != 0)
		attached = true;
	else if (AllocConsole() != 0)
		allocated = true;
	if (attached || allocated)
	{
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
		if (allocated)
			_console = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	return fission::Success;
}

int fatal_error(fission::string error, fission::string message, source_location location)
{
	NOT_USED(error, message, location);
	using namespace fission;
	scratch_arena.reset();
    //printf("\x1b[91m%.*s\x1b[0m: %.*s (\x1b[92m%s\x1b[0m in \x1b[93m%s:%i\x1b[0m)\n",
    //    (int)error.count, (char*)error.data, (int)message.count, (char*)message.data,
    //    location.function, location.file, location.line);
	MessageBoxW(0, L"Hi", L"julie <3", MB_OK|MB_ICONERROR);
	ExitProcess(1);
}

LRESULT Window::_setup_callback(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) noexcept
{
	using namespace fission;

    if (Msg == WM_NCCREATE) {
        auto p_create_struct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto p_window = reinterpret_cast<Window*>(p_create_struct->lpCreateParams);

        if (p_window == nullptr) {
            log::error("Failed to receive window [WM_NCCREATE]");
            goto Default_Procedure;
        }

        log::verbose("Setting up message callback");

        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(p_window));
        SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(_message_callback));
    }
Default_Procedure:
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

END_NAMESPACE()

BEGIN_NAMESPACE(fission)

void Engine::run() {
	{
		scoped_set(logging_prefix, OS_NAME);
    	log::verbose("Starting message loop...");
	}

    BOOL bRet {};
    MSG msg;
    while ((bRet = GetMessageW(&msg, nullptr, 0, 0)) != 0) {
        if (bRet == -1) {
        	log::error("An Error Occured!"
                       "(but idk what kind of error, "
                       "this never happens to me, I swear)");
            engine.exit_code = EXIT_FAILURE;
            break;
        }
        else {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    // invalidate main window
    engine.window._handle = NULL;
}

#define WINDOW_CLASS_NAME L"Fission-Engine"

struct Window_Style {
    DWORD value, ex;
};

auto Window::create (Create_Info const& info) -> Result
{
	NOT_USED(info);
	scoped_set(logging_prefix, OS_NAME);
    log::info("Creating window...");

    HINSTANCE instance = GetModuleHandleW(nullptr);

    WNDCLASSEXW window_class_info = {
        .cbSize = sizeof(WNDCLASSEXW),
        .lpfnWndProc = Window::_setup_callback,
        .hInstance = instance,
        .hIcon = LoadIcon(NULL, IDI_SHIELD),
        .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
        .lpszClassName = WINDOW_CLASS_NAME,
    };

    if (!RegisterClassExW(&window_class_info)) {
    //    report_error("RegisterClassExW", "Failed to register window class");
        return Failed;
    }
    else log::verbose("Registered window class");

    Window_Style style {
        WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_THICKFRAME | WS_VISIBLE,
        0
    };

    auto title = L"Julie<3";//win32_to_wide_string(info.title);

    _handle = CreateWindowExW(
        style.ex,            // Ex Style
        WINDOW_CLASS_NAME,   // Window Class Name
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

    return Success;
}

END_NAMESPACE()
