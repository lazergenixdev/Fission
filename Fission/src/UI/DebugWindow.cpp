#include "DebugWindow.h"
#if FISSION_ENABLE_DEBUG_UI

#include <Fission/Core/Application.hh>

namespace Fission
{
	DebugWindow::DebugWindow(): 
		neutron::DynamicWindow( { 0, 300, 0, 500 }, Flags_None )
	{}

	void DebugWindow::OnCreate( FApplication * app )
	{
		CreateRenderer2D( &m_pRenderer2D );
		m_pRenderer2D->OnCreate( app->pGraphics, Rect.size() );

		FontManager::SetFont( "$ui", "../resources/Fonts/Noto Sans/NotoSans-Regular.ttf", 14.0f, app->pGraphics );

		context.r2d = m_pRenderer2D.get();
		context.rect = base::rectf( Rect );
	}

	void DebugWindow::OnUpdate( timestep dt )
	{
		auto r2d = m_pRenderer2D.get();
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
		//auto tl = r2d->DrawString( "Test Slider", { 12.0f, 30.0f }, Colors::make_gray( 0.8f ) );
		//r2d->FillRect( { 120.0f, 292.0f, 30.0f, 30.0f + tl.height }, (rgb_colorf)Colors::make_gray<rgb_color8>( 33 ) );
		//r2d->DrawString( "2.04", { 120.0f + 2.0f, 30.0f }, Colors::make_gray( 0.8f ) );

		float offsetY = rect.top() + 20.0f;

		std::erase_if( m_Widgets,
			[] ( WidgetAndLabel & wal ) { 
				bool remove = !wal.second->active;
				if( remove ) delete wal.second; 
				return remove;
			} 
		);

		for( auto && widget : m_ActiveWidgets )
		{
			widget->OnUpdate( dt, &offsetY, &context );
			widget->active = false;
		}

		m_ActiveWidgets.clear();

		r2d->Render();
	}

	DebugWidget * DebugWindow::GetWidget( const char * label, int WidgetID, const char * format )
	{
#define RETURN(ptr) m_ActiveWidgets.emplace_back(ptr); return ptr
		auto widget = m_Widgets.find(label);

		// widget was found
		if( widget != m_Widgets.end() )
		{
			// TODO: here we should check if it is of the right type.
			widget->second->active = true;
			RETURN( widget->second );
		}

		WidgetAndLabel pair = std::make_pair( label, nullptr );

		// no widget was found, we need to create it.
		switch( WidgetID )
		{
		case DebugSlider<float>::ID:
		{
			pair.second = new DebugSlider<float>( pair.first.c_str(), format );
			// status = contains information about the insertion.
			auto status = m_Widgets.insert( pair );
			// .first   = iterator of where the widget was inserted;
			// ->second = pointer to the widget
			RETURN( status.first->second );
		}

		default:
			break;
		}

		return nullptr;
#undef RETURN
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

#endif