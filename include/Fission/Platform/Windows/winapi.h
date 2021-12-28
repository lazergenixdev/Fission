#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
//#define NOGDICAPMASKS
//#define NOSYSMETRICS
#define NOMENUS
//#define NOICONS
//#define NOKEYSTATES
//#define NORASTEROPS
//#define NOSHOWWINDOW
//#define NOATOM
//#define NOCLIPBOARD
//#define NOCOLOR
//#define NOCTLMGR
#define NODRAWTEXT
//#define NOGDI
//#define NOKERNEL
//#define NONLS
//#define NOMEMMGR
//#define NOMETAFILE
//#define NOOPENFILE
#define NOSCROLL
//#define NOSOUND
//#define NOTEXTMETRIC
//#define NOWH
//#define NOCOMM
//#define NOKANJI
//#define NOHELP
//#define NOPROFILER
//#define NODEFERWINDOWPOS
//#define NOMCX

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


#include <wrl/client.h>

// Win32 Helpers
namespace Fission::Platform
{
	template <typename T>
	using com_ptr = Microsoft::WRL::ComPtr<T>;

	// HMODULE wrapper that will automatically free the handle as it exits scope.
	struct Module
	{
		Module( const char * _Module_Name ) : m_hModule( GetModuleHandleA(_Module_Name) ) {}

		Module( HMODULE hModule ) : m_hModule( hModule ) {}
		~Module() { if( m_hModule ) FreeModule( m_hModule ); }

		explicit Module( HMODULE& ) = delete;
		explicit Module( HMODULE&& ) = delete;

		HMODULE operator*() { return m_hModule; }
		operator HMODULE() { return m_hModule; }

		bool is_null() const { return m_hModule == NULL; }

	private:
		HMODULE m_hModule;
	};
}
