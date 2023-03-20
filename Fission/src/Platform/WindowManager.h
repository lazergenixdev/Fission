#pragma once
#include <Fission/Core/Window.hh>

namespace Fission
{
	struct WindowManager : public ManagedObject
	{
		virtual void Initialize() = 0;

		virtual void SetGraphics( Graphics * pGraphics ) = 0;

		virtual void CreateWindow( const Window::CreateInfo * pInfo, Window ** ppWindow ) = 0;
	};

	extern void CreateWindowManager( WindowManager ** ppWindowManager ) noexcept;
}