#pragma once
#include "Fission/Platform/System.h"
#include "WindowsMonitor.h"

#ifdef FISSION_PLATFORM_WINDOWS

#include "DXErr/dxerr.hpp"
#include <thread>

#include <shellapi.h>
#pragma comment(lib, "shell32")

namespace Fission {

	void System::Initialize()
	{
		HRESULT hr;

		// Initialize COM
		if( FAILED( hr = CoInitializeEx( nullptr, COINIT_SPEED_OVER_MEMORY | COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE ) ) )
		{
			static char errorbuffer[512];
			DXGetErrorDescriptionA( hr, errorbuffer, std::size( errorbuffer ) );
			throw exception( "COM Error", _lazer_exception_msg.append( "Failed to Initialize COM", errorbuffer ) );
		}

		Platform::EnumMonitors();
	}

	void System::Shutdown()
	{
		CoUninitialize();
	}

	void System::DisplayMessageBox( const std::wstring & title, const std::wstring & text )
	{
		auto _show_message = [] ( const wchar_t * title, const wchar_t * text ) {
			MessageBoxW( NULL, text, title, MB_OK | MB_SYSTEMMODAL );
		};

		auto _msg_thread = std::thread( _show_message, title.c_str(), text.c_str() );

		_msg_thread.join();
	}

	bool System::OpenURL( const std::string & _URL )
	{
		return (bool)ShellExecuteA( NULL, "open", _URL.c_str(), nullptr, nullptr, SW_SHOWDEFAULT );
	}

	bool System::OpenFile( const file::path & _File )
	{
		return (bool)ShellExecuteW( NULL, L"explore", _File.c_str(), nullptr, nullptr, SW_SHOWDEFAULT );
	}

}

#endif /* FISSION_PLATFORM_WINDOWS */