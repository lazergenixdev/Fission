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
#define NONLS               // - All NLS defines and routines
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

#include <windows.h>

// STOP DOING THIS WINDOWS, I DON'T WANT YOUR STUPID MACROS
#undef CreateWindow
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
