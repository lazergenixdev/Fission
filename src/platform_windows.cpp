#include "Fission/core.hpp"
#include <Windows.h>
#include <Windowsx.h>
#include <uxtheme.h>
#include <dwmapi.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Dwmapi.lib")

// --------------------------------------------------------------------------------
// Globals

static LARGE_INTEGER perf_frequency;

// --------------------------------------------------------------------------------

// I WANT BETTER WINDOWS!
//#define TEST

LRESULT CALLBACK _message_callback(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
#ifdef TEST
    case WM_NCCALCSIZE:
        // Remove default non-client frame completely
        if (wParam) return 0;
        break;
    case WM_NCHITTEST: {
		constexpr int TITLEBAR_HEIGHT = 40;
		constexpr int RESIZE_BORDER = 8;
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd, &pt);

        RECT rc;
        GetClientRect(hwnd, &rc);

        // Resize borders
        if (pt.y < RESIZE_BORDER) {
            if (pt.x < RESIZE_BORDER) return HTTOPLEFT;
            if (pt.x > rc.right - RESIZE_BORDER) return HTTOPRIGHT;
            return HTTOP;
        }
        if (pt.y > rc.bottom - RESIZE_BORDER) {
            if (pt.x < RESIZE_BORDER) return HTBOTTOMLEFT;
            if (pt.x > rc.right - RESIZE_BORDER) return HTBOTTOMRIGHT;
            return HTBOTTOM;
        }
        if (pt.x < RESIZE_BORDER) return HTLEFT;
        if (pt.x > rc.right - RESIZE_BORDER) return HTRIGHT;

        // Custom draggable titlebar
        if (pt.y < TITLEBAR_HEIGHT) return HTCAPTION;

        return HTCLIENT;
    }
#endif

#ifdef SINGLE_THREAD
	case WM_ENTERSIZEMOVE:
		SetTimer(hwnd, 0, 1, NULL);
	break;
	case WM_EXITSIZEMOVE:
		KillTimer(hwnd, 0);
	break;
	case WM_TIMER:
	//	log::info("timer!!!");
	//	engine.flags |= engine.Graphics_Recreate_Swap_Chain;
		engine.render_frame();
	//	SetTimer(hwnd, 0, 1, NULL);
	break;
	case WM_MOVING:
	case WM_SIZING: {
		if (Msg == WM_SIZING)
			engine.flags |= engine.Graphics_Recreate_Swap_Chain;
		engine.render_frame();
	//	SetTimer(hwnd, 0, 1, NULL);
	} break;
	case WM_SIZE: {
	//	log::info("SIZE");
	} break;
#endif

	case WM_SYSKEYDOWN: case WM_SYSKEYUP:
	{
        if(wParam != VK_MENU && (wParam < VK_F1 || VK_F24 < wParam || wParam == VK_F4))
        {
			break;
        }
	} // fallthrough;
	case WM_KEYDOWN: case WM_KEYUP:
	{
        bool was_down = (lParam & (1<<30));
        bool is_down  = !(lParam & (1<<31));
        
        bool is_repeat = false;
		bool release = false;
        if(!is_down)
			release = true;
        else if(was_down)
			is_repeat = true;
        
        bool right_sided = false;
        if ((lParam & (1<<24)) &&
            (wParam == VK_CONTROL || wParam == VK_RCONTROL ||
             wParam == VK_MENU || wParam == VK_RMENU ||
             wParam == VK_SHIFT || wParam == VK_RSHIFT))
			right_sided = true;
        
		auto& event = engine.window.event_queue[engine.window.event_tail];
		event.type = release ? Event_Key_Up : Event_Key_Down;
		event.key_down.key_id = u32(wParam);
		engine.window.event_tail = (engine.window.event_tail + 1) % array_count(engine.window.event_queue);
        // repeat_count = lParam & 0xFFFF;
	} break;

    case WM_CLOSE: {
        log::verbose("Got message requesting to close window");
        engine.flags &=~ Engine::Running;
		return DefWindowProcW(hwnd, Msg, wParam, lParam);
    }

    case WM_DESTROY: {
        engine.flags &=~ Engine::Running;
        PostQuitMessage(0);
		return DefWindowProcW(hwnd, Msg, wParam, lParam);
    }
    }
	return DefWindowProcW(hwnd, Msg, wParam, lParam);
    //auto p_window = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    //return p_window->process_message(hWnd, Msg, wParam, lParam);
}

namespace fission
{
	auto utf8_to_16(Arena& arena, string in, bool null_terminate = true) -> string_u16
	{
		c16* start = arena.next_ptr<c16>();
		size_t i = 0;

		next: while (i < in.count) {
			c8 c = in.data[i];
			c32 codepoint = 0;
			int extra_bytes = 0;

			if (c <= 0x7F) { // 1-byte (ASCII)
				codepoint = c;
				extra_bytes = 0;
			} else if ((c >> 5) == 0x6) { // 2-byte
				codepoint = c & 0x1F;
				extra_bytes = 1;
			} else if ((c >> 4) == 0xE) { // 3-byte
				codepoint = c & 0x0F;
				extra_bytes = 2;
			} else if ((c >> 3) == 0x1E) { // 4-byte
				codepoint = c & 0x07;
				extra_bytes = 3;
			} else {
				// Error: Invalid UTF-8 start byte
				{ i++; continue; }
			}

			// Error: Truncated UTF-8 sequence
			if (i + extra_bytes >= in.count)
				break;

			for (int j = 0; j < extra_bytes; ++j) {
				c8 cc = in.data[i + j + 1];
				// Error: Invalid UTF-8 continuation byte
				if ((cc >> 6) != 0x2)
					{ i++; goto next; }
				codepoint = (codepoint << 6) | (cc & 0x3F);
			}

			i += extra_bytes + 1;

			// Encode as UTF-16
			if (codepoint <= 0xFFFF) {
				arena.push(static_cast<c16>(codepoint));
			} else {
				codepoint -= 0x10000;
				auto high = static_cast<c16>(0xD800 + (codepoint >> 10));
				auto low  = static_cast<c16>(0xDC00 + (codepoint & 0x3FF));
				arena.push(high);
				arena.push(low);
			}
		}

		if (null_terminate)
			arena.push<c16>(0);
		return {size_t(arena.next_ptr<c16>() - start), start};
	}
}

BEGIN_NAMESPACE(os)

auto init() -> fission::Result
{
	// Try and get a handle to stdout
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

	// OS Information
	SYSTEM_INFO system_info {};
	GetNativeSystemInfo(&system_info);
	_info.page_size = system_info.dwPageSize;

	QueryPerformanceFrequency(&perf_frequency);
	
	return fission::Success;
}

int fatal_error(string error, string message, source_location location)
{
	using namespace fission;
	using namespace formatting;
	scratch_arena.reset();
	string text = format(scratch_arena, error, ": ",
		message, "\n\n(", location.function,
		" in ", location.file, ":", location.line, ")", null);
	//! TODO: use UTF-16 version instead of ASCII
	MessageBoxA(0, text.cstr(), "Fatal Error", MB_OK|MB_ICONERROR);
	ExitProcess(1);
}

LRESULT Window::_setup_callback(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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

auto ticks() -> u64
{
	LARGE_INTEGER perf;
	QueryPerformanceCounter(&perf);
	return u64(perf.QuadPart);
}

auto seconds_elasped_and_reset(u64& t) -> f64
{
	auto now = ticks();
	auto dt = now - t;
	t = now;
	return f64(dt) / f64(perf_frequency.QuadPart);
}

void Engine::run()
{
	{
		scoped_set(logger.prefix, OS_NAME);
    	log::verbose("Starting message loop...");
	}

    BOOL bRet {};
    MSG msg;
	#ifndef SINGLE_THREAD
    while ((bRet = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1) {
			scoped_set(logger.prefix, OS_NAME);
        	log::error("GetMessage returned -1! I've literally never seen this before!");
            engine.exit_code = EXIT_FAILURE;
            break;
        }
        else {
			TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
	#else
	engine.setup();
    for (;engine.flags & engine.Running;)
    {
        if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) != 0)
        {
		//	log::info("window message: ", msg.message);
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        else
        {
			engine.render_frame();
        }
    }
	#endif
	
    // invalidate window
    engine.window._handle = NULL;
}

#define WINDOW_CLASS_NAME L"Fission-Engine"

auto Window::create (Create_Info const& info) -> Result
{
	scoped_set(logger.prefix, OS_NAME);
    log::info("Creating window...");

	// Do dumb copy, so that temp_arena gets "freed" automatically
	auto win32_arena = engine.temp_arena;

    HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSEXW window_class_info = {
        .cbSize = sizeof(WNDCLASSEXW),
		.style = CS_VREDRAW|CS_HREDRAW,
        .lpfnWndProc = _message_callback,//Window::_setup_callback,
        .hInstance = instance,
        .hIcon = LoadIcon(NULL, IDI_SHIELD),
	 	.hCursor = LoadCursorA(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
        .lpszClassName = WINDOW_CLASS_NAME,
    };

    if (!RegisterClassExW(&window_class_info)) {
    	log::error("RegisterClassExW", "Failed to register window class");
        return Failed;
    }
    else log::verbose("Registered window class");

    auto title = utf8_to_16(win32_arena, info.title);

    _handle = CreateWindowExW(
        WS_EX_APPWINDOW,
        WINDOW_CLASS_NAME,
        title.wstr(),
#ifdef TEST
        WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
#else
		WS_OVERLAPPEDWINDOW | WS_SIZEBOX,
#endif
        100,
        100,
        info.width,
        info.height,
        NULL, NULL,
        instance,
        this
    );
#ifdef TEST
    // Extend frame into client area fully (-1) to remove default edges
    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(_handle, &margins);
#endif
	ShowWindow(_handle, SW_SHOW);

    return Success;
}

END_NAMESPACE()
