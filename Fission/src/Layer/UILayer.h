#pragma once
#include "LazerEngine/Core/Layer.h"

namespace lazer::ui {
	class WindowManager;
}

namespace lazer {

	class UILayer : public IUILayer
	{
	public:
		UILayer();

		virtual void OnCreate();
		virtual void OnUpdate();

		virtual EventResult OnKeyDown( KeyDownEventArgs & args );
		virtual EventResult OnKeyUp( KeyUpEventArgs & args );
		virtual EventResult OnTextInput( TextInputEventArgs & args );
		virtual EventResult OnMouseMove( MouseMoveEventArgs & args );
		virtual EventResult OnMouseLeave( MouseLeaveEventArgs & args );
		virtual EventResult OnSetCursor( SetCursorEventArgs & args );
	};

}

