#include "Debug.h"
#if FISSION_ENABLE_DEBUG_UI
#include "DebugWindow.h"

namespace Fission
{
	static struct DebugWindow * s_DebugWindow;

	namespace UI
	{
		bool Debug::SliderFloat( const char * label, float * value, const char * format )
		{


			return false;
		}
	}


	void CreateDebug( WindowManager * pWindowManager, FApplication * app )
	{
		s_DebugWindow = new DebugWindow;

		IFWindow::CreateInfo info = {};
		info.pEventHandler = s_DebugWindow;
		info.wProperties.style = IFWindow::Style::Borderless;
		info.wProperties.size = s_DebugWindow->Rect.size();
		pWindowManager->CreateWindow( &info, &s_DebugWindow->window );

		s_DebugWindow->OnCreate( app );
	}

	void RenderDebug( timestep dt )
	{
		auto SwapChain = s_DebugWindow->window->GetSwapChain();

		SwapChain->Bind();
		s_DebugWindow->OnUpdate( dt );
		SwapChain->Present( vsync_Off );
	}

	void DestroyDebug()
	{
		s_DebugWindow->Destroy();
	}
}

#endif // FISSION_ENABLE_DEBUG_UI
