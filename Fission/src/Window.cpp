#include "Fission/Core/Window.h"
#include "Fission/Core/Configuration.h"

#ifdef FISSION_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsWindow.h"
#endif

namespace Fission {
	
	std::unique_ptr<Window> Window::Create( const Properties & props, Graphics * pGraphics, IEventHandler * event_handler )
	{
#ifdef FISSION_PLATFORM_WINDOWS
		return std::make_unique<Platform::WindowsWindow>( props, pGraphics, event_handler );
#endif // FISSION_PLATFORM_WINDOWS
	}

}