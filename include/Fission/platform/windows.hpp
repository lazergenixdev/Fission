/**
 *	______________              _____
 *	___  ____/__(_)________________(_)____________
 *	__  /_   __  /__  ___/_  ___/_  /_  __ \_  __ \
 *	_  __/   _  / _(__  )_(__  )_  / / /_/ /  / / /
 *	/_/      /_/  /____/ /____/ /_/  \____//_/ /_/
 *
 *
 * @Author:       lazergenixdev@gmail.com
 * @Development:  (https://github.com/lazergenixdev/Fission)
 * @License:      MIT (see end of file)
 */
#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS       // - CC_*, LC_*, PC_*, CP_*, TC_*, RC_
//#define NOVIRTUALKEYCODES // - VK_*
//#define NOWINMESSAGES     // - WM_*, EM_*, LB_*, CB_*
//#define NOWINSTYLES       // - WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
//#define NOICONS           // - IDI_*
//#define NOSYSCOMMANDS     // - SC_*
//#define NOSHOWWINDOW      // - SW_*
//#define NOATOM            // - Atom Manager routines
//#define NOCLIPBOARD       // - Clipboard routines
//#define NOCTLMGR          // - Control and Dialog routines
//#define NOGDI             // - All GDI defines and routines
//#define NOUSER            // - All USER defines and routines
//#define NOMB              // - MB_* and MessageBox()
//#define NOMSG             // - typedef MSG and associated routines
//#define NOWINOFFSETS      // - GWL_*, GCL_*, associated routines
#define NOOPENFILE          // - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NODEFERWINDOWPOS    // - DeferWindowPos routines
//#define NONLS               // - All NLS defines and routines
#define NOSYSMETRICS        // - SM_*
#define NOMENUS             // - MF_*
#define NOKEYSTATES         // - MK_*
#define NORASTEROPS         // - Binary and Tertiary raster ops
#define OEMRESOURCE         // - OEM Resource values
#define NOCOLOR             // - Screen colors
#define NODRAWTEXT          // - DrawText() and DT_*
#define NOKERNEL            // - All KERNEL defines and routines
#define NOMEMMGR            // - GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE          // - typedef METAFILEPICT
#define NOMINMAX            // - Macros min(a,b) and max(a,b)
#define NOSCROLL            // - SB_* and scrolling routines
#define NOSERVICE           // - All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND             // - Sound driver routines
#define NOTEXTMETRIC        // - typedef TEXTMETRIC and associated routines
#define NOWH                // - SetWindowsHook and WH_*
#define NOCOMM              // - COMM driver routines
#define NOKANJI             // - Kanji support stuff.
#define NOHELP              // - Help engine interface.
#define NOPROFILER          // - Profiler interface.
#define NOMCX               // - Modem Configuration Extensions

// TODO: I hope to replace <Windows.h> with raw function definitions
#include <Windows.h>

// STOP DOING THIS WINDOWS, I DON'T WANT YOUR STUPID MACROS
#undef CreateWindow // <- this is the worst
#undef LoadLibrary
#undef LoadCursor
#undef MessageBox
#undef GetMessage
#undef SendMessage
#undef DispatchMessage
#undef DefWindowProc
#undef RegisterClass
#undef UnregisterClass
#undef SetWindowText
#undef GetWindowLongPtr
#undef SetWindowLongPtr
#undef GetMonitorInfo

#define VK_USE_PLATFORM_WIN32_KHR 1
#define FISSION_PLATFORM_VULKAN_EXTENSION_NAMES \
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME

#define OS_CALL WINAPI

namespace os
{
// ----------------------- Mutexes ------------------------

using Mutex = HANDLE;
#define os_mutex_create(p_mutex) *(p_mutex) = CreateMutexW(nullptr, FALSE, nullptr)
#define os_mutex_destroy(mutex)  CloseHandle(mutex)
#define os_mutex_lock(mutex)     (WaitForSingleObject(mutex, INFINITE) == WAIT_FAILED)
#define os_mutex_unlock(mutex)   (ReleaseMutex(mutex) == 0)


// ----------------------- Threads ------------------------

using Thread = HANDLE;
using Thread_Result = DWORD;
#define os_thread_start(function, p_arg, p_thread) \
    ((*(p_thread) = CreateThread(nullptr, 0, function, p_arg, 0, nullptr)) == NULL)
#define os_thread_join(thread) \
	WaitForSingleObject(thread, INFINITE); \
    CloseHandle(thread);

}

#define _os_main() \
int APIENTRY ::WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)

#include <Fission/config.hpp>
__FISSION_BEGIN__

namespace platform {
    //enum Window_ {
    //    Window_Disable_Position_Update = 1 << 0,
    //    Window_Minimized               = 1 << 1,
    //    Window_Init_Completed          = 1 << 2,
    //    Window_Enable_Mouse_Deltas     = 1 << 3,
    //};
	struct Window
    {
    public:
		HWND _handle {};
		int  _mouse_wheel_delta {};
        u32  _flags {};

    protected:
        static LRESULT CALLBACK _setup_callback(HWND, UINT, WPARAM, LPARAM) noexcept;
	};

	struct Display
    {
		HMONITOR _handle {};
	};
}

__FISSION_END__
