#pragma once
#include "Fission/Core/Scene.h"

namespace Fission {

	class UILayer : public IUILayer
	{
	public:
		UILayer();

		virtual void OnCreate();
		virtual void OnUpdate();

		virtual EventResult OnMouseMove( MouseMoveEventArgs & args ) override;
		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;
		virtual EventResult OnKeyUp( KeyUpEventArgs & args ) override;
		virtual EventResult OnSetCursor( SetCursorEventArgs & args ) override;
		virtual EventResult OnTextInput( TextInputEventArgs & args ) override;
		virtual EventResult OnMouseLeave( MouseLeaveEventArgs & args ) override;
	};

}

