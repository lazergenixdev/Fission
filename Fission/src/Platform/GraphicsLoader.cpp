#include "GraphicsLoader.h"

#ifdef FISSION_PLATFORM_WINDOWS
#include "Windows/WinGraphicsLoader.h"
namespace Fission
{
	void CreateGraphicsLoader( GraphicsLoader ** loader )
	{
		*loader = new Platform::WindowsGraphicsLoader;
	}
}
#elif 0
#endif