#include "WinWindowManager.h"
#include <Fission/Base/Exception.h>
#include "WindowsWindow.h"

namespace Fission::Platform
{
	using PFN_GET_STOCK_OBJECT = decltype( &::GetStockObject );

	void WindowsWindowManager::Initialize()
	{
		m_Info.hInstance = GetModuleHandleW(nullptr);

		WNDCLASSEXW wClassDesc = {};
		wClassDesc.cbSize = sizeof( WNDCLASSEXW );
		wClassDesc.lpszClassName =  m_Info.WindowClassName;
		wClassDesc.hInstance =      m_Info.hInstance;
		wClassDesc.lpfnWndProc =    SetupWindowsProc;

		HMODULE hGDI = LoadLibraryA( "gdi32" );
		if( hGDI != NULL )
		{
			auto GetStockObj = (PFN_GET_STOCK_OBJECT)GetProcAddress( hGDI, "GetStockObject" );
			if( GetStockObj != NULL )
				wClassDesc.hbrBackground = (HBRUSH)GetStockObj(BLACK_BRUSH);
			FreeLibrary( hGDI );
		}

		RegisterClassExW( &wClassDesc );
	}

	void WindowsWindowManager::SetGraphics( IFGraphics * pGraphics )
	{
		m_Info.pGraphics = pGraphics;
	}

	void WindowsWindowManager::CreateWindow( const IFWindow::CreateInfo * pInfo, IFWindow ** ppWindow )
	{
		*ppWindow = new WindowsWindow( pInfo, &m_Info );
	}

	void WindowsWindowManager::Destroy()
	{
		UnregisterClassW( m_Info.WindowClassName, m_Info.hInstance );
		delete this;
	}
}