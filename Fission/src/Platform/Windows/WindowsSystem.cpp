#pragma once
#include "Fission/Platform/System.h"
#include "Fission/Base/Exception.h"
#include "WindowsMonitor.h"

#ifdef FISSION_PLATFORM_WINDOWS

#include "DXErr/dxerr.hpp"
#include <thread>

#include <shellapi.h>
#pragma comment(lib, "shell32")

namespace Fission {

	namespace detail
	{
		class WindowsVersion
		{
		public:
			inline const char * c_str() { return _Version; }

			static WindowsVersion & Get()
			{
				static WindowsVersion ver;
				return ver;
			}
		private:
			WindowsVersion()
			{
				// TODO: Throw proper exceptions if an error occurs, even if it is very unlikely.

				typedef LONG NTSTATUS;
				typedef NTSTATUS( WINAPI * RtlGetVersionPtr )( PRTL_OSVERSIONINFOW );

				RTL_OSVERSIONINFOW version = { 0 };
				version.dwOSVersionInfoSize = sizeof( version );

				// Since `GetVersion()` depends on your application manifest,
				// we need to import another function to get the real OS version
				HMODULE h_ntdll = ::GetModuleHandleA( "ntdll" );
				if( h_ntdll == NULL ) throw 0x45;

				RtlGetVersionPtr RtlGetVersion = reinterpret_cast<RtlGetVersionPtr>( GetProcAddress(h_ntdll,"RtlGetVersion") );
				if( RtlGetVersion == nullptr ) throw 0x45;

				if( 0 != RtlGetVersion( &version ) ) throw 0x45;

			#ifdef FISSION_PLATFORM_WINDOWS32
				BOOL IsRunningOn64 = FALSE;
				if( FALSE == IsWow64Process( GetCurrentProcess(), &IsRunningOn64 ) ) throw 0x45;
				sprintf_s(_Version,"Windows 10 %i-Bit (%u.%u.%u)",IsRunningOn64?64:32,version.dwMajorVersion,version.dwMinorVersion,version.dwBuildNumber);
			#else // FISSION_PLATFORM_WINDOWS64
				sprintf_s(_Version,"Windows 10 64-Bit (%u.%u.%u)",version.dwMajorVersion,version.dwMinorVersion,version.dwBuildNumber);
			#endif
			}

			char _Version[64];
		};
	}

	void System::ShowSimpleMessageBox(
		const string & _Title,
		const string & _Text,
		const MessageBoxFlag_t & _Flags,
		IFWindow * _Parent_Window
	)
	{
		// Convert strings from UTF-8 to UTF-16
		auto title = _Title.utf16();
		auto text = _Text.utf16();

		HWND hwnd = NULL;
		if( _Parent_Window )
			hwnd = _Parent_Window->native_handle();

		UINT flags = MB_OK | MB_SYSTEMMODAL;
		switch( _Flags & 0x7 )
		{
		case MessageBoxFlags::Error:   flags |= MB_ICONERROR;       break;
		case MessageBoxFlags::Warning: flags |= MB_ICONWARNING;     break;
		case MessageBoxFlags::Info:    flags |= MB_ICONINFORMATION; break;
		default:break;
		}

		auto _show_message = [&] () {
			MessageBoxW( hwnd, (LPCWSTR)text.c_str(), (LPCWSTR)title.c_str(), flags );
		};

		// This is needed because of quirks with the rules of what thread message boxes 
		//   can be shown on, so we just bypass by making our own thread
		auto _msg_thread = std::thread( _show_message );

		// Block the thread until user exits the message box
		_msg_thread.join();
	}

	bool System::OpenURL( const std::filesystem::path & _URL )
	{
		return (bool)ShellExecuteW( NULL, L"open", _URL.c_str(), nullptr, nullptr, SW_SHOWDEFAULT );
	}

	bool System::OpenFileLocation( const std::filesystem::path & _File )
	{
		return (bool)ShellExecuteW( NULL, L"explore", _File.c_str(), nullptr, nullptr, SW_SHOWDEFAULT );
	}

	const char * System::GetVersionString()
	{
		return detail::WindowsVersion::Get().c_str();
	}
}

#endif /* FISSION_PLATFORM_WINDOWS */