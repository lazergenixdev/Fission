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

	void System::Initialize()
	{
		HRESULT hr;

		// Initialize COM
		if( FAILED( hr = CoInitializeEx( nullptr, COINIT_SPEED_OVER_MEMORY | COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE ) ) )
		{
			static char errorbuffer[512];
			DXGetErrorDescriptionA( hr, errorbuffer, std::size( errorbuffer ) );
			FISSION_THROW( "COM Error",.append( "Failed to Initialize COM", errorbuffer ) );
		}

		Platform::EnumMonitors();
	}

	void System::Shutdown()
	{
		CoUninitialize();
	}

	void System::DisplayMessageBox( const string & title, const string & text )
	{
		auto _show_message = [] ( const wchar_t * title, const wchar_t * text ) {
			MessageBoxW( NULL, text, title, MB_OK | MB_SYSTEMMODAL );
		};

		auto _Title = title.utf16();
		auto _Text = text.utf16();
		auto _msg_thread = std::thread( _show_message, (LPWSTR)_Title.c_str(), (LPWSTR)_Text.c_str() );

		_msg_thread.join();
	}

	bool System::OpenURL( const string & _URL )
	{
		return (bool)ShellExecuteA( NULL, "open", _URL.c_str(), nullptr, nullptr, SW_SHOWDEFAULT );
	}

	bool System::OpenFile( const std::filesystem::path & _File )
	{
		return (bool)ShellExecuteW( NULL, L"explore", _File.c_str(), nullptr, nullptr, SW_SHOWDEFAULT );
	}

}

#endif /* FISSION_PLATFORM_WINDOWS */