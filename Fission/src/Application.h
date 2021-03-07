#pragma once
#include "Fission/Core/Application.h"

#include "Layer/UILayer.h"
#include "Layer/DebugLayer.h"
#include "Layer/ConsoleLayer.h"
#include "Layer/ImGuiLayer.h"

namespace Fission {

	struct ApplicationState {

		std::unique_ptr<Window> m_pMainWindow;
		std::unique_ptr<Graphics> m_pGraphics;

		ConsoleLayer m_ConsoleLayer;
		DebugLayerImpl m_DebugLayer;
#ifndef IMGUI_DISABLE
		ImGuiLayer m_ImGuiLayer;
#endif
		UILayer m_UILayer;
		std::vector<ILayer *> m_vMainLayers;

		bool m_bRunning = false;
		bool m_bMinimized = false;

		std::condition_variable m_MinimizeCondition;

		bool m_bRecreate = false;

		Platform::ExitCode m_ExitCode = 0;

	};

}