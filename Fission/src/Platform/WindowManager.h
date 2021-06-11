#pragma once
#include <Fission/Core/Window.hh>

namespace Fission
{
	struct WindowManager
	{
		virtual void Initialize() = 0;

		virtual void SetGraphics( IFGraphics * pGraphics ) = 0;

		virtual void CreateWindow( const IFWindow::CreateInfo * pInfo, IFWindow ** ppWindow ) = 0;

		virtual void Destroy() = 0;
	};

	extern void CreateWindowManager( WindowManager ** ppWindowManager ) noexcept;
}