#include "GraphicsLoader.h"

#ifdef FISSION_PLATFORM_WINDOWS
#include "Windows/WinGraphicsLoader.h"
#endif

namespace Fission
{
	void CreateGraphicsLoader( GraphicsLoader ** loader )
	{
#ifdef FISSION_PLATFORM_WINDOWS
#include "Windows/WinGraphicsLoader.h"
		*loader = new Platform::WindowsGraphicsLoader;
#elif 0
#endif
	}
}