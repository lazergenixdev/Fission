#include "WindowManager.h"

#ifdef FISSION_PLATFORM_WINDOWS
#include "Windows/WinWindowManager.h"
namespace Fission
{
	void CreateWindowManager( WindowManager ** manager ) noexcept
	{
		*manager = new Platform::WindowsWindowManager;
	}
}
#elif 0
#endif