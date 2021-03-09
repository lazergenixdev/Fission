#pragma once
#include "Fission/Core/Layer.h"

namespace Fission {

	class ImGuiLayer : public ILayer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnCreate();
		virtual void OnUpdate();

		virtual EventResult OnKeyDown( KeyDownEventArgs & ) override;
		virtual EventResult OnKeyUp( KeyUpEventArgs & ) override;
		virtual EventResult OnTextInput( TextInputEventArgs & ) override;
		virtual EventResult OnMouseMove( MouseMoveEventArgs & ) override;
		virtual EventResult OnMouseLeave( MouseLeaveEventArgs & ) override;
		virtual EventResult OnSetCursor( SetCursorEventArgs & ) override;
		virtual EventResult OnClose( CloseEventArgs & ) override;

	private:
		bool m_bExiting = false;
		bool m_bCreated = false;
		std::mutex m_ExitMutex;
	};

}
