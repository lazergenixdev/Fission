//#pragma once
//#include "Fission/Core/Application.h"
//#include "SceneStack.h"
//
//#include "Layer/UILayer.h"
//#include "Layer/DebugLayer.h"
//#include "Layer/ConsoleLayer.h"
//
//namespace Fission {
//
//	struct ApplicationState {
//
//		std::unique_ptr<Window>		pMainWindow;
//		std::unique_ptr<Graphics>	pGraphics;
//
//		SceneStack					SceneStack;
//
//		ConsoleLayer				ConsoleLayer;
//		DebugLayerImpl				DebugLayer;
//		UILayer						UILayer;
//
//		bool						bRunning = false;
//		bool						bMinimized = false;
//
//		std::mutex					PauseMutex;
//		std::condition_variable		PauseCondition;
//
//		bool						bRecreate = false;
//		bool						bReadyToExit = false;
//
//		Platform::ExitCode			ExitCode = 0;
//
//
//		ApplicationState( const FApplication::CreateInfo & info )
//			: SceneStack( info.startScene )
//		{
//
//		}
//
//	};
//
//}