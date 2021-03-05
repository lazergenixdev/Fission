#pragma once
#include "Fission/Core/Layer.h"

namespace Fission {

	class ImGuiLayer : public ILayer
	{
	public:
		ImGuiLayer();

		virtual void OnCreate();
		virtual void OnUpdate();

		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;
	};

}
