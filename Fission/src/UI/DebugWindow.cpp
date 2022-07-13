#include "DebugWindow.h"
#if FISSION_ENABLE_DEBUG_UI

#include <Fission/Core/Application.hh>

namespace NotoSansRegularTTF
{
#include "Static Fonts/NotoSans-Regular.inl"
}

namespace Fission
{
	DebugWindow::DebugWindow(): 
		neutron::DynamicWindow( { 0, 300, 0, 500 }, Flags_None )
	{}

	void DebugWindow::OnCreate( FApplication * app )
	{
		CreateRenderer2D( &m_pRenderer2D );
		m_pRenderer2D->OnCreate( app->f_pGraphics, Rect.size() );

		FontManager::SetFont( "$ui", NotoSansRegularTTF::data, NotoSansRegularTTF::size, 14.0f, app->f_pGraphics );

		context.r2d = m_pRenderer2D.get();
		context.rect = rf32( Rect );
	}

	void DebugWindow::OnUpdate( timestep dt )
	{
		auto r2d = m_pRenderer2D.get();
		float offsetY;
		{
			std::unique_lock lock(mutex);

			rf32 rect = { 0.0f,300.0f,0.0f,500.0f };

			r2d->SelectFont( FontManager::GetFont( "$ui" ) );
			float border = 2.0f;

			r2d->DrawRect( rect, Colors::Black, border, StrokeStyle::Inside );

			r2d->FillRect( rect.expand( -border ), Colors::make_gray( 0.18f ) );

			rect.x.low += 70.0f;
			rect.y.high = rect.y.low + 20.0f;
			r2d->FillRect( rect, (rgb_colorf)Colors::make_gray<rgb_color8>( 19 ) );

			r2d->DrawString( "Debug", { 12.0f, 3.0f }, Colors::make_gray( 0.9f ) );

			offsetY = rect.top() + 20.0f;

			// get rid of any widgets that are not being used.
			std::erase_if( m_Widgets,
				[] ( WidgetAndLabel & wal ) { 
					bool remove = !wal.second->active;
					if( remove ) delete wal.second; 
					return remove;
				} 
			);

		}

		for( auto && widget : m_ActiveWidgets )
		{
			widget->OnUpdate( dt, &offsetY, &context );
			widget->active = false;
		}

		m_ActiveWidgets.clear();
		for( auto && widget : m_TempWidgets )
			delete widget;
		m_TempWidgets.clear();

		r2d->Render();
	}

	inline DebugWidget * DebugWindow::InsertWidget( const char * label, DebugWidget * widget )
	{
		auto pair = std::make_pair( label, widget );
		pair.second->parent = this;
		m_Widgets.emplace( pair );
		return widget;
	}
	inline DebugWidget * DebugWindow::InsertTempWidget( DebugWidget * widget )
	{
		m_TempWidgets.emplace_back( widget );
		return widget;
	}

	DebugWidget * DebugWindow::GetWidget( const char * label, int WidgetID, const char * format, const void * pvalue )
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

		// no widget was found, we need to create it.
		switch( WidgetID )
		{
		case DebugSlider<float>::ID: {RETURN( InsertWidget( label, new DebugSlider<float>( label, format, *(float*)pvalue ) ) );}
		case DebugSlider<int>::ID:   {RETURN( InsertWidget( label, new DebugSlider<int>  ( label, format, *(int*)pvalue ) ) );}
		case DebugText::ID:          {RETURN( InsertTempWidget( new DebugText( label ) ) );}
		case DebugButton::ID:        {RETURN( InsertWidget( label, new DebugButton( label ) ) );}
		case DebugCheckbox::ID:      {RETURN( InsertWidget( label, new DebugCheckbox( label, *(bool*)pvalue ) ) );}
		case DebugInput<float>::ID:  {RETURN( InsertWidget( label, new DebugInput<float>( label, format, *(float*)pvalue ) ) );}
		case DebugInput<int>::ID:    {RETURN( InsertWidget( label, new DebugInput<int>  ( label, format, *(int*)pvalue ) ) );}

		default:
			break;
		}

		return nullptr;
#undef RETURN
	}


	void DebugWindow::SetCapture( Window * window ) { capture = window; }


	EventResult DebugWindow::OnMouseMove( MouseMoveEventArgs & args )
	{
		std::unique_lock lock( mutex );

		neutron::MouseMoveEventArgs nargs;
		nargs.pos = args.position;
		neutron::g_MousePosition = args.position;

		if( capture ) {
			return (EventResult)capture->OnMouseMove( nargs );
		}
		else 
		{
			for( auto && [label,w] : m_Widgets )
				if( w->isInside( args.position - Rect.topLeft() ) )
				{
					if( hover ) hover->OnMouseLeave();
					hover = w;
					w->OnMouseMove( nargs );
					return EventResult::Handled;
				}
			if( hover ) hover->OnMouseLeave();
			hover = nullptr;
		}

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
	EventResult DebugWindow::OnKeyDown( KeyDownEventArgs & args )
	{
		neutron::KeyDownEventArgs nargs;
		nargs.key = args.key;

		if( args.key == Keys::Control )
			g_KeyboardState.ctrl_down = true;

		if( args.key == Keys::Mouse_Left || args.key == Keys::Mouse_Right ) {
			if( hover ) {
				if( focus && focus != hover ) focus->OnFocusLost();
				focus = hover;
				focus->OnFocusGain();
				hover->OnKeyDown( nargs );
				return EventResult::Handled;
			}
		}
		else if( focus ) {
			focus->OnKeyDown( nargs );
			return EventResult::Handled;
		}
		if( focus ) {
			focus->OnFocusLost();
			focus = nullptr;
		}
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
		neutron::KeyUpEventArgs nargs;
		nargs.key = args.key;

		if( args.key == Keys::Control )
			g_KeyboardState.ctrl_down = false;

		if( focus )
			return (EventResult)focus->OnKeyUp( nargs );

		if( mousedown && args.key == Keys::Mouse_Left )
			mousedown = false;

		return EventResult::Handled;
	}
	EventResult DebugWindow::OnTextInput( TextInputEventArgs & args )
	{
		neutron::TextInputEventArgs nargs;
		nargs.ch = args.codepoint;

		if( focus ) focus->OnTextInput( nargs );

		return EventResult::Handled;
	}

	void DebugWindow::Destroy() { delete this; }
}

#endif