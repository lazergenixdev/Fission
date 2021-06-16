#pragma once
#include <Fission/Core/Scene.hh>

#define _lazer_has_vector
#define _lazer_has_point
#define _lazer_has_rect
	namespace react::ui {
		template <typename T> using vector = std::vector<T>;
		using point = Fission::base::vector2i;
		using rect = Fission::base::recti;
	}
#define _lazer_char_type			wchar_t
#define _lazer_key_type				Fission::Keys::Key
#define _lazer_key_left_mouse_		Fission::Keys::Mouse_Left
#define _lazer_key_right_mouse_		Fission::Keys::Mouse_Right
#define _lazer_cursor_type			Fission::Cursor *
#include <Fission/reactui.h>

namespace Fission {

	class UILayer : public IFUILayer
	{
	public:
		UILayer();

	//	virtual void OnCreate();
		virtual void OnUpdate();

		virtual EventResult OnMouseMove( MouseMoveEventArgs & args ) override;
		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;
		virtual EventResult OnKeyUp( KeyUpEventArgs & args ) override;
		virtual EventResult OnSetCursor( SetCursorEventArgs & args ) override;
		virtual EventResult OnTextInput( TextInputEventArgs & args ) override;
		virtual EventResult OnMouseLeave( MouseLeaveEventArgs & args ) override;

	private:
		// Removes any widgets or windows that should not be visible.
		void RemoveInActive();

		// Creates all the windows and widgets.
		void CreateActive();

	private:
	//	scoped<react::ui::WindowManager> pWindowManager;
	};

}

