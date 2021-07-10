#pragma once
#include <Fission/Core/UI/Debug.hh>
#include "../Platform/WindowManager.h"

namespace Fission
{
#if FISSION_ENABLE_DEBUG_UI
	extern void CreateDebug( WindowManager * pWindowManager, FApplication * app );
	extern void RenderDebug( timestep dt );
#else
	#define CreateDebug( ... )
	#define RenderDebug( ... )
#endif
}