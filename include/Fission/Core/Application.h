#pragma once
#include "Fission/config.h"
#include "Window.h"
#include "Graphics/Graphics.h"
#include "Layer.h"

namespace Fission {

	class Application : public IEventHandler
	{
	public:
		struct CreateInfo
		{
			Window::Properties wndProps = {};
			Graphics::API gAPI = Graphics::API::Default;
			vec2i resolution = { 1280, 720 };
		};

	public:
		virtual void OnCreate() = 0;

	public:
		Application( const Application & ) = delete;

		FISSION_API Application( const CreateInfo & fallback = {} );

		FISSION_API virtual ~Application() noexcept;

		FISSION_API static Application * Get();

	public:

		FISSION_API void PushLayer( const char * name, ILayer * layer );
		
		FISSION_API DebugLayer * GetDebugLayer();
		
		FISSION_API void Exit( Platform::ExitCode ec = 0 );
		
		FISSION_API void Recreate();
		
		FISSION_API Window * GetWindow();
		
		FISSION_API Graphics * GetGraphics();
		
	//	FISSION_API basic_timer * GetTimer();
		
	//	FISSION_API void SetGraphicsProperties( Graphics::API gAPI );
		
		FISSION_API Platform::ExitCode Run();


		FISSION_API EventResult OnKeyDown     ( KeyDownEventArgs & )         override;

		FISSION_API EventResult OnKeyUp       ( KeyUpEventArgs & )           override;

		FISSION_API EventResult OnTextInput   ( TextInputEventArgs & )       override;

		FISSION_API EventResult OnMouseMove    ( MouseMoveEventArgs & )      override;

		FISSION_API EventResult OnMouseLeave   ( MouseLeaveEventArgs & )     override;

		FISSION_API EventResult OnSetCursor    ( SetCursorEventArgs & )      override;

		FISSION_API EventResult OnHide         ()                            override;

		FISSION_API EventResult OnShow         ()                            override;

		FISSION_API EventResult OnClose        ( CloseEventArgs & )          override;

	private:
		struct ApplicationState * m_State;

	}; // class Fission::Application

} // namespace Fission
