#pragma once
#include <Fission/Core/UI/Debug.hh>
#include "../Platform/WindowManager.h"

namespace Fission
{
#if FISSION_ENABLE_DEBUG_UI
	extern void CreateDebug( WindowManager * pWindowManager, FApplication * app );
	extern void RenderDebug( timestep dt );
	extern void DestroyDebug();
#else
	#define CreateDebug( ... ) (void)0
	#define RenderDebug( ... ) (void)0
	#define DestroyDebug()     (void)0
#endif
}