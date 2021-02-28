#include "Fission/Core/Window.h"
#include "Fission/Core/Configuration.h"

#ifdef FISSION_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsWindow.h"
#endif

namespace lazer {
	
	std::unique_ptr<Window> Window::Create( const Properties & props, IEventHandler * event_handler ) 
	{
		Properties wp = props;
		if( bool( wp.flags & Window::Flags::IsMainWindow ) )
		{
			wp = Configuration::GetWindowConfig( wp );
			wp.title = props.title;
		}

#ifdef FISSION_PLATFORM_WINDOWS
		return std::make_unique<Platform::WindowsWindow>( wp, event_handler );
#endif // FISSION_PLATFORM_WINDOWS
	}

}