#include "WinWindowManager.h"
#include <Fission/Base/Exception.h>
#include "WindowsWindow.h"
#include "../Monitor.h"

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
		wClassDesc.lpfnWndProc =    WindowsProcSetup;

		// if this doesn't work, no big deal. It just means we don't get black as the startup window color.
		if( auto gdi = Module("gdi32") )
		{
			if( auto GetStockObj = gdi.Get<PFN_GET_STOCK_OBJECT>("GetStockObject") )
			{
				wClassDesc.hbrBackground = static_cast<HBRUSH>( GetStockObj(BLACK_BRUSH) );
			}
		}

		RegisterClassExW( &wClassDesc );

		EnumMonitors();
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