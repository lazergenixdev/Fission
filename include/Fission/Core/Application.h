#pragma once
#include "LazerEngine/config.h"
#include "Window.h"
#include "Graphics/Graphics.h"
#include "Layer.h"

namespace lazer {

	class FrameTimer {
	public:
		LAZER_API static float GetLastDelta();
	};

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

		LAZER_API Application( const CreateInfo & fallback = {} );

		LAZER_API virtual ~Application() noexcept;

		LAZER_API static Application * Get();

	public:

		LAZER_API void PushLayer( const char * name, ILayer * layer );
		
		LAZER_API DebugLayer * GetDebugLayer();
		
		LAZER_API void Exit( Platform::ExitCode ec = 0 );
		
		LAZER_API void Recreate();
		
		LAZER_API Window * GetWindow();
		
		LAZER_API Graphics * GetGraphics();
		
	//	LAZER_API basic_timer * GetTimer();
		
	//	LAZER_API void SetGraphicsProperties( Graphics::API gAPI );
		
		LAZER_API Platform::ExitCode Run();


		LAZER_API EventResult OnKeyDown		( KeyDownEventArgs & )		override;

		LAZER_API EventResult OnKeyUp		( KeyUpEventArgs & )		override;

		LAZER_API EventResult OnTextInput	( TextInputEventArgs & )	override;

		LAZER_API EventResult OnMouseMove	( MouseMoveEventArgs & )	override;

		LAZER_API EventResult OnMouseLeave	( MouseLeaveEventArgs & )	override;

		LAZER_API EventResult OnSetCursor	( SetCursorEventArgs & )	override;

		LAZER_API EventResult OnHide		()							override;

		LAZER_API EventResult OnShow		()							override;

		LAZER_API EventResult OnClose		( CloseEventArgs & )		override;

	private:
		struct ApplicationState * m_State;

	};

}
