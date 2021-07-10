#include "Debug.h"
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Application.hh>

namespace Fission
{
	static struct Test * _s_test;
	static struct IFWindow * _s_test_window;

	struct Test : public IFLayer
	{
		std::mutex mutex;
		IFRenderer2D * r2d;

		POINT last = {};
		bool mousedown = false;
		base::vector2i mousepos;

		virtual void OnCreate( FApplication * app ) override
		{
			CreateRenderer2D( &r2d );
			r2d->OnCreate( app->pGraphics, { 300, 500 } );

			FontManager::SetFont( "$ui", "../resources/Fonts/Noto Sans/NotoSans-Regular.ttf", 14.0f, app->pGraphics );
		}
		virtual void OnUpdate( timestep dt ) override
		{
			base::rectf rect = {0.0f,300.0f,0.0f,500.0f};

			r2d->SelectFont( FontManager::GetFont( "$ui" ) );
			float border = 2.0f;

			r2d->DrawRect(rect,Colors::Black,border,StrokeStyle::Inside);

			r2d->FillRect( rect.expand( -border ),Colors::make_gray(0.18f));

			rect.x.low += 70.0f;
			rect.y.high = rect.y.low + 20.0f;
			r2d->FillRect( rect,(rgb_colorf)Colors::make_gray<rgb_color8>(19));

			r2d->DrawString( "Debug", { 12.0f, 3.0f }, Colors::make_gray(0.9f) );

			// psuedo slider to see how it looks
			auto tl = r2d->DrawString( "Test Slider", { 12.0f, 30.0f }, Colors::make_gray(0.8f) );
			r2d->FillRect( { 120.0f, 292.0f, 30.0f, 30.0f + tl.height }, (rgb_colorf)Colors::make_gray<rgb_color8>(33) );
			r2d->DrawString( "2.04", { 120.0f + 2.0f, 30.0f }, Colors::make_gray(0.8f) );

			r2d->Render();
		}

		virtual EventResult OnSetCursor( SetCursorEventArgs & args ) override
		{
			args.cursor = Cursor::Get( Cursor::Default_Move );
			return EventResult::Handled;
		}
		virtual EventResult OnMouseMove( MouseMoveEventArgs & args ) override
		{
			mousepos = args.position;
			if( mousedown )
			{
				POINT currentpos;
				GetCursorPos( &currentpos );
				int x = currentpos.x - last.x;
				int y = currentpos.y - last.y;
				MoveWindow( args.native_event->hWnd, x, y, 300, 500, false );
				return EventResult::Handled;
			}
			return EventResult::Pass;
		}
		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override
		{
			if( args.key == Keys::Mouse_Left )
			{
				mousedown = true;
				GetCursorPos( &last );
				RECT rect;
				GetWindowRect( args.native_event->hWnd, &rect );
				last.x = last.x - rect.left;
				last.y = last.y - rect.top;
			}
			return EventResult::Handled;
		}
		virtual EventResult OnKeyUp( KeyUpEventArgs & args ) override
		{
			if( args.key == Keys::Mouse_Left )
				mousedown = false;
			return EventResult::Handled;
		}

		virtual void Destroy() override { delete this; }

	};

#if FISSION_ENABLE_DEBUG_UI
	void CreateDebug( WindowManager * pWindowManager, FApplication * app )
	{
		_s_test = new Test;
		IFWindow::CreateInfo info = {};
		info.pEventHandler = _s_test;
		info.wProperties.style = IFWindow::Style::Borderless;
		info.wProperties.size = { 300,500 };
		pWindowManager->CreateWindow( &info, &_s_test_window );

		_s_test->OnCreate( app );
	}

	void RenderDebug( timestep dt )
	{
		_s_test_window->GetSwapChain()->Bind();
		_s_test->OnUpdate(dt);
		_s_test_window->GetSwapChain()->Present( vsync_Off );
	}
#endif
}