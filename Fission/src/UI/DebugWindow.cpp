#include "DebugWindow.h"

#include <Fission/Core/Application.hh>

namespace Fission
{
	DebugWindow::DebugWindow(): 
		neutron::DynamicWindow( { 0, 300, 0, 500 }, Flags_None )
	{}

	void DebugWindow::OnCreate( FApplication * app )
	{
		CreateRenderer2D( &r2d );
		r2d->OnCreate( app->pGraphics, Rect.size() );

		FontManager::SetFont( "$ui", "../resources/Fonts/Noto Sans/NotoSans-Regular.ttf", 14.0f, app->pGraphics );
	}

	void DebugWindow::OnUpdate( timestep dt )
	{
		base::rectf rect = { 0.0f,300.0f,0.0f,500.0f };

		r2d->SelectFont( FontManager::GetFont( "$ui" ) );
		float border = 2.0f;

		r2d->DrawRect( rect, Colors::Black, border, StrokeStyle::Inside );

		r2d->FillRect( rect.expand( -border ), Colors::make_gray( 0.18f ) );

		rect.x.low += 70.0f;
		rect.y.high = rect.y.low + 20.0f;
		r2d->FillRect( rect, (rgb_colorf)Colors::make_gray<rgb_color8>( 19 ) );

		r2d->DrawString( "Debug", { 12.0f, 3.0f }, Colors::make_gray( 0.9f ) );

		// psuedo slider to see how it looks
		auto tl = r2d->DrawString( "Test Slider", { 12.0f, 30.0f }, Colors::make_gray( 0.8f ) );
		r2d->FillRect( { 120.0f, 292.0f, 30.0f, 30.0f + tl.height }, (rgb_colorf)Colors::make_gray<rgb_color8>( 33 ) );
		r2d->DrawString( "2.04", { 120.0f + 2.0f, 30.0f }, Colors::make_gray( 0.8f ) );

		r2d->Render();
	}


	EventResult DebugWindow::OnMouseMove( MouseMoveEventArgs & args )
	{
		neutron::MouseMoveEventArgs nargs;
		nargs.pos = args.position;
		if( DynamicWindow::OnMouseMove( nargs ) )
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
		return EventResult::Handled;
	}
	EventResult DebugWindow::OnKeyDown( KeyDownEventArgs & args )
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
	EventResult DebugWindow::OnKeyUp( KeyUpEventArgs & args )
	{
		if( args.key == Keys::Mouse_Left )
			mousedown = false;
		return EventResult::Handled;
	}

	void DebugWindow::Destroy() { delete this; }
}