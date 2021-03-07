#pragma once
#include "Fission/Core/Layer.h"
#include <queue>

namespace Fission {

	class ImGuiLayer : public ILayer
	{
	public:
		virtual void OnCreate();
		virtual void OnUpdate();

		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;
		virtual EventResult OnKeyUp( KeyUpEventArgs & ) override;
		virtual EventResult OnTextInput( TextInputEventArgs & ) override;
		virtual EventResult OnMouseMove( MouseMoveEventArgs & ) override;
		virtual EventResult OnMouseLeave( MouseLeaveEventArgs & ) override;
		virtual EventResult OnSetCursor( SetCursorEventArgs & ) override;
	private:
		std::queue<Platform::Event> m_EventQueue;
		bool m_bRender = false;
	};

}
