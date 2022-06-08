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
		Module( HMODULE hModule ) noexcept : m_hModule( hModule ) {}
		Module( const char * _Module_Name ) : m_hModule( GetModuleHandleA(_Module_Name) ) {}

		static Module Load( const char *_Lib_Name ) noexcept { return { LoadLibraryA(_Lib_Name) }; }

		~Module() { if( m_hModule ) FreeModule( m_hModule ); }

		Module( Module& ) = delete;
		Module( Module &&src ) : m_hModule( src.m_hModule ) { src.m_hModule = NULL; }

		constexpr operator HMODULE() const noexcept { return m_hModule; }
		constexpr operator bool() const noexcept { return m_hModule != NULL; }

		constexpr bool is_null() const noexcept { return m_hModule == NULL; }

		template <typename F>
		F Get( const char *name ) const noexcept { return reinterpret_cast<F>( GetProcAddress(m_hModule,name) ); }

	private:
		HMODULE m_hModule;
	};
}
