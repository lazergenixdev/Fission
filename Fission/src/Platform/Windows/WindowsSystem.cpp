#pragma once
#include "Fission/Platform/System.h"
#include "Fission/Base/Exception.hpp"
#include "WindowsMonitor.h"

#ifdef FISSION_PLATFORM_WINDOWS

#include "DXErr/dxerr.hpp"
#include <thread>

#include <shellapi.h>
#pragma comment(lib, "shell32")

namespace Fission {

	namespace impl
	{
		class WindowsVersion
		{
		public:
			operator const char*() { return _Version; }

			static WindowsVersion & Get()
			{
				static WindowsVersion ver;
				return ver;
			}
		private:
			WindowsVersion()
			{
				typedef LONG NTSTATUS;
				typedef NTSTATUS( WINAPI * pfn_RtlGetVersion )( PRTL_OSVERSIONINFOW );
				
				pfn_RtlGetVersion RtlGetVersion;

				RTL_OSVERSIONINFOW version = { 0 };
				version.dwOSVersionInfoSize = sizeof( version );

				// Since `GetVersion()` depends on your application manifest,
				// we need to import another function to get the real OS version
				auto ntdll = Platform::Module( "ntdll" );

				if( ntdll.is_null() )
					goto error;

				// Get function address
				RtlGetVersion = ntdll.Get<pfn_RtlGetVersion>("RtlGetVersion");

				if( RtlGetVersion == nullptr )
					goto error;

				// Retrieve the true windows version for this machine.
				if( 0 != RtlGetVersion( &version ) )
					goto error;

			#ifdef FISSION_PLATFORM_WINDOWS32
				BOOL IsRunningOn64 = FALSE;
				if( FALSE == IsWow64Process( GetCurrentProcess(), &IsRunningOn64 ) ) throw FISSION_THROW("Win32 Error","IsWow64Process() returned false");
				sprintf_s(_Version,"Windows %i %i-Bit (%u.%u.%u)",version.dwMajorVersion,IsRunningOn64?64:32,version.dwMajorVersion,version.dwMinorVersion,version.dwBuildNumber);
			#else // FISSION_PLATFORM_WINDOWS64
				sprintf_s(_Version,"Windows %i 64-Bit (%u.%u.%u)",version.dwMajorVersion,version.dwMajorVersion,version.dwMinorVersion,version.dwBuildNumber);
			#endif
				return;

			error:
				sprintf_s(_Version,"Windows (Unknown Version)");

			}

			char _Version[64];
		};
	}

	void System::ShowSimpleMessageBox(
		string_view const& _Title,
		string_view const& _Text,
		const MessageBoxFlag_t & _Flags,
		IFWindow * _Parent_Window
	)
	{
		// Convert strings from UTF-8 to UTF-16
		auto title = _Title.as<utf16>();
		auto text = _Text.as<utf16>();

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

	void System::ShowNativeMessageBox(
		const platform_char* _Title,
		const platform_char* _Text,
		const MessageBoxFlag_t& _Flags,
		IFWindow* _Parent_Window
	)
	{
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

		auto _show_message = [&]() {
			MessageBoxW( hwnd, _Text, _Title, flags );
		};

		// This is needed because of quirks with the rules of what thread message boxes 
		//   can be shown on, so we just bypass by making our own thread
		auto _msg_thread = std::jthread( _show_message );
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
		return impl::WindowsVersion::Get();
	}
}

#endif /* FISSION_PLATFORM_WINDOWS */