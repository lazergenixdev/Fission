﻿#include "UILayer.h"
#include <Fission/Core/UI/UI.h>
#include <Fission/Core/Application.h>

// Everything about this code is awful, please avert your eyes for your own well being. You have been warned.

using namespace Fission;

static scoped<Renderer2D>			g_pRenderer2D;
static std::mutex					g_Mutex;

namespace NunitoSemiBoldTTF
{
#include <Static Fonts/Nunito-SemiBold.inl>
}


enum WidgetFlags
{
	WidgetFlags_None = 0x0,

	WidgetFlags_Changed = 0x1,
};

enum class WidgetType
{
#define TYPE /* This is a type (IDE helper) */
#define SUBTYPE /* This is a sub type (IDE helper) */

#define MAKE_SUBTYPE(N) (N<<26)

	/*
	* 
	// USAGE:
	
	// Getting Sub-Type:
		switch( _type & mask_subtype_bits )
		{
		case type_input_number: { break; }
		case type_input_string: { break; }
		...
		};

	*/

	mask_type_bits		= 0b000000'11111111111111111111111111,
	mask_subtype_bits	= ~mask_type_bits,


	/*==========================================================*/
	// For these types `data3` is used to store the numeric value
	SUBTYPE type_input_number = MAKE_SUBTYPE(1),

	TYPE input_float		= type_input_number | 0x0,
	TYPE input_int			= type_input_number | 0x1,
	TYPE input_int64		= type_input_number | 0x2,
	TYPE input_float64		= type_input_number | 0x3,

	/*==========================================================*/
	// Not implemented
	SUBTYPE type_toggle = MAKE_SUBTYPE(2),

	TYPE checkbox			= type_toggle | 0x0,

	/*==========================================================*/
	// Not implemented
	SUBTYPE type_input_string = MAKE_SUBTYPE(3),

	TYPE input_string		= type_input_string | 0x0,
	TYPE input_ml_string	= type_input_string | 0x1,


#undef MAKE_SUBTYPE

#undef SUBTYPE
#undef TYPE
};

struct WidgetInfo
{
	std::string name;
	int use_count = 0;
	ui::window_uid uid = ui::null_window_uid;

	// Internal Flags
	WidgetType type;
	int Flags = WidgetFlags_None;

	// Pointer storage
	void * data1;
	void * data2;

	// Flags & Number storage
	uint64_t data3;
	uint64_t data4;
};

struct WindowInfo
{
	std::string name;
	int use_count = 0;
	ui::window_uid uid = ui::null_window_uid;
	std::unordered_map<std::string,WidgetInfo> widgets;
};

struct FallbackWindowInfo : public WindowInfo
{};

static std::unordered_map<std::string, WindowInfo> g_WindowContext;
static WindowInfo * g_pActiveWindow;
static FallbackWindowInfo g_FallbackWindow = {"Debug"};


namespace Fission
{
	EventResult UILayer::OnMouseMove( MouseMoveEventArgs & args )
	{
		ui::MouseMoveEventArgs m;
		m.pos = args.position;
		std::scoped_lock lock( g_Mutex );
		return (EventResult)pWindowManager->OnMouseMove( m );
	}
	EventResult UILayer::OnKeyDown( KeyDownEventArgs & args )
	{
		ui::KeyDownEventArgs m;
		m.key = args.key;
		std::scoped_lock lock( g_Mutex );
		return (EventResult)pWindowManager->OnKeyDown( m );
	}
	EventResult UILayer::OnKeyUp( KeyUpEventArgs & args )
	{
		ui::KeyUpEventArgs m;
		m.key = args.key;
		std::scoped_lock lock( g_Mutex );
		return (EventResult)pWindowManager->OnKeyUp( m );
	}
	EventResult UILayer::OnTextInput( TextInputEventArgs & args )
	{
		ui::TextInputEventArgs m;
		m.ch = args.character;
		std::scoped_lock lock( g_Mutex );
		return (EventResult)pWindowManager->OnTextInput( m );
	}
	EventResult UILayer::OnSetCursor( SetCursorEventArgs & args )
	{
		lazer::ui::SetCursorEventArgs m;
		m.cursor = args.cursor;
		std::scoped_lock lock( g_Mutex );
		EventResult r = (EventResult)pWindowManager->OnSetCursor( m );
		args.cursor = m.cursor;
		return r;
	}
	EventResult UILayer::OnMouseLeave( MouseLeaveEventArgs & args )
	{
		std::scoped_lock lock( g_Mutex );
		return (EventResult)pWindowManager->OnMouseLeave();
	}
}

static color g_ColorBackground = Colors::Gray_80_Percent;

class DogeWindow : public lazer::ui::DynamicWindow
{
public:
	DogeWindow( std::string label ) : DynamicWindow( { 100, 212, 100, 258 } ), id(label) {}
	virtual lazer::ui::Result OnSetCursor( lazer::ui::SetCursorEventArgs & args ) override
	{
		if( DynamicWindow::OnSetCursor( args ) )
		{
			auto pos = lazer::ui::GetRectPos( Rect, lazer::ui::g_MousePosition, 8, 0, 8 );
			switch( pos )
			{
			case lazer::ui::rect_pos_left:			args.cursor = Cursor::Get( Cursor::Default_SizeX ); break;
			case lazer::ui::rect_pos_right:			args.cursor = Cursor::Get( Cursor::Default_SizeX ); break;
			case lazer::ui::rect_pos_top:			args.cursor = Cursor::Get( Cursor::Default_SizeY ); break;
			case lazer::ui::rect_pos_bottom:		args.cursor = Cursor::Get( Cursor::Default_SizeY ); break;
			case lazer::ui::rect_pos_top_left:		args.cursor = Cursor::Get( Cursor::Default_SizeTLBR ); break;
			case lazer::ui::rect_pos_top_right:		args.cursor = Cursor::Get( Cursor::Default_SizeBLTR ); break;
			case lazer::ui::rect_pos_bottom_left:	args.cursor = Cursor::Get( Cursor::Default_SizeBLTR ); break;
			case lazer::ui::rect_pos_bottom_right:	args.cursor = Cursor::Get( Cursor::Default_SizeTLBR ); break;
			default: break;
			}
		}
		return lazer::ui::Handled;
	}

	virtual void OnUpdate( float dt ) override
	{
		rectf rect = (rectf)Rect;

		auto tl = g_pRenderer2D->CreateTextLayout( id.c_str() );

		g_pRenderer2D->FillRect( rect, g_ColorBackground );
		g_pRenderer2D->FillRect( rectf::from_tl({rect.get_l()+60.0f,rect.get_t()},{rect.width()-60.0f,tl.height}), Colors::Gray_90_Percent );
		
		g_pRenderer2D->DrawString( id.c_str(), rect.get_tl()+vec2f((60.0f-tl.width)*0.5f,0.0f), Colors::Snow );

		g_pRenderer2D->DrawRect( rect, Colors::Black, 2.0f, StrokeStyle::Outside );

		g_pRenderer2D->PushTransform( mat3x2f::Translation( rect.get_tl() ) );
		DynamicWindow::OnUpdate( dt );
		g_pRenderer2D->PopTransform();
	}
public:
	float offsetY = 14.0f;
private:
	std::string id;
};

class Slider : public lazer::ui::Slider
{
public:
	static constexpr int s_PaddingX = 5;
	static constexpr int s_PaddingY = 5;

	Slider( std::string label, int window_width, int startY, WidgetInfo * widget ) : ui::Slider(), pWidget(widget) {
		auto tl = g_pRenderer2D->CreateTextLayout( L"A" ); // LOL
		auto y = startY + s_PaddingY;
		Rect = { s_PaddingX,window_width - s_PaddingX,y,y +(int)tl.height };
		this->label = ( label );
		this->numberText = std::to_string( (float&)widget->data3 );
	}

	void drag_number( float value )
	{
		(float &)pWidget->data3 = value;
		pWidget->Flags |= WidgetFlags_Changed;
		this->numberText = std::to_string( value );
	}
	void resolve_number()
	{
		float original = (float&)pWidget->data3;
		try { original = std::stof( numberText ); }
		catch( ... ) { return; }

		(float &)pWidget->data3 = original;
		pWidget->Flags |= WidgetFlags_Changed;
		this->numberText = std::to_string( original );
	}

	virtual void OnFocusLost() override
	{
		resolve_number();
	}

	virtual lazer::ui::Result OnMouseMove( lazer::ui::MouseMoveEventArgs & args ) override
	{
		if( dragging )
		{
			vec2i offset = args.pos - startDrag;
			drag_number( startNumber + float(offset.x/2)*0.1f );
			return lazer::ui::Handled;
		}

		rect dragRect = { Rect.get_l(), Rect.x.get_average(), Rect.get_t(), Rect.get_b() };

		int center = Rect.x.get_average();
		int pos = ui::GetMousePosition().x - parent->Rect.get_l();

		inTextBox = ( pos > center );

		return lazer::ui::Handled;
	}
	virtual lazer::ui::Result OnMouseLeave() override
	{
		inTextBox = false;
		return lazer::ui::Handled;
	}

	virtual lazer::ui::Result OnKeyDown( lazer::ui::KeyDownEventArgs & args ) override
	{
		switch( args.key )
		{
		case Keys::Mouse_Left:
		{
			if( !this->inTextBox )
			{
				dragging = true;
				startDrag = ui::GetMousePosition();
				startNumber = std::stof( numberText );
				parent->SetCapture( this );
			}
			break;
		}
		default: break;
		}
		return lazer::ui::Handled;
	}
	virtual lazer::ui::Result OnKeyUp( lazer::ui::KeyUpEventArgs & args ) override
	{
		switch( args.key )
		{
		case Keys::Mouse_Left:
		{
			if( dragging )
			{
				dragging = false;
				parent->SetFocus( nullptr );
				parent->SetCapture( nullptr );
			}
			break;
		}
		default: break;
		}
		return lazer::ui::Handled;
	}

	virtual lazer::ui::Result OnTextInput( lazer::ui::TextInputEventArgs & args ) override
	{
		switch( args.ch )
		{
		case '\b': if( numberText.size() ) numberText.pop_back(); break;
		case '\r': resolve_number(); parent->SetFocus( nullptr ); break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '.':
			numberText += (char)args.ch; break;
		}
		return lazer::ui::Handled;
	}

	virtual lazer::ui::Result OnSetCursor( lazer::ui::SetCursorEventArgs & args ) override
	{
		rect dragRect = { Rect.get_l(), Rect.x.get_average(), Rect.get_t(), Rect.get_b() };

		int center = Rect.x.get_average();
		int pos = ui::GetMousePosition().x - parent->Rect.get_l();

		if( pos < center )
			args.cursor = Cursor::Get( Cursor::Default_Arrow );
		else
			args.cursor = Cursor::Get( Cursor::Default_TextInput );


		return lazer::ui::Pass;
	}

	virtual void OnParentResize( ui::rect r ) override
	{
		Rect.x.high = r.width() - s_PaddingX;
	}

	virtual bool isInside( ui::point p ) override
	{
		return Rect[p];
	}

	virtual void OnUpdate( float dt ) override
	{
		rectf rect = (rectf)Rect;

		g_pRenderer2D->DrawString( label.c_str(), rect.get_tl(), this == parent->GetHover() ? Colors::White : Colors::Gray_30_Percent );

		rectf numberbox = { rect.x.get_average(), rect.get_r(), rect.get_t(), rect.get_b() };

		g_pRenderer2D->FillRect( numberbox, Colors::Gray_85_Percent );

		if( inTextBox ) selectAlpha += ( 0.35f - selectAlpha ) * dt * 5.0f;
		else			selectAlpha += ( 0.00f - selectAlpha ) * dt * 5.0f;

		if( selectAlpha > 0.0f )
		g_pRenderer2D->DrawRect( numberbox, color(1.0f, 1.0f, 1.0f, selectAlpha), 1.0f, StrokeStyle::Outside );

		auto tl = g_pRenderer2D->DrawString( numberText.c_str(), numberbox.get_tl(), Colors::White );

		if( this == parent->GetFocus() && not dragging )
			g_pRenderer2D->FillRect(
				rectf::from_center( vec2f( (float)(int)(numberbox.get_l() + tl.width) + 0.5f, numberbox.y.get_average() ), 1.0f, tl.height-4.0f ),
				Colors::White );
	}
private:
	std::string label;
	std::string numberText;
	recti Rect;
	bool inTextBox = false;
	float selectAlpha = 0.0f;

	bool dragging = false;
	vec2i startDrag = {};
	float startNumber = 0.0f;

	WidgetInfo * pWidget;
};


namespace Fission::UI {

	inline WindowInfo * GetActiveWindow() { 
		if( g_pActiveWindow ) { ++g_pActiveWindow->use_count; return g_pActiveWindow; } ++g_FallbackWindow.use_count; return &g_FallbackWindow;
	}

	inline void Reset() { if( g_pActiveWindow ) g_pActiveWindow = nullptr; }


	bool Debug::Window( const char * label )
	{
		if( !label ) { g_pActiveWindow= &g_FallbackWindow; ++g_FallbackWindow.use_count; return true; }

		std::string sLabel = label;
		auto it = g_WindowContext.find( sLabel );
		if( it == g_WindowContext.end() )
		{
			g_WindowContext.emplace( sLabel, sLabel );
			g_pActiveWindow = &g_WindowContext[sLabel];
			++g_pActiveWindow->use_count;
			return true;
		}

		g_pActiveWindow = &it->second;
		++g_pActiveWindow->use_count;
		return true;
	}

	//bool Debug::Button( const char * label )
	//{
	//	auto window = GetActiveWindow();

	//	std::string sLabel = label;
	//	auto it = window->widgets.find( sLabel );
	//	if( it == window->widgets.end() )
	//	{
	//		window->widgets.emplace( sLabel, sLabel );
	//	}

	//	return true;
	//}

	bool Debug::InputFloat( const char * label, float * value )
	{
		auto window = GetActiveWindow();

		std::string sLabel = label;
		auto it = window->widgets.find( sLabel );
		if( it == window->widgets.end() ) // If this widget does not exist, then create it's context
		{
			WidgetInfo widget;
			widget.name = sLabel;
			widget.type = WidgetType::input_float;
			(float&)widget.data3 = *value; // don't store the pointer, we don't own this value!
			widget.use_count = 1;
			window->widgets.emplace( sLabel, widget );
			return false;
		}

		++it->second.use_count;
		if( it->second.Flags & WidgetFlags_Changed )
		{
			it->second.Flags = WidgetFlags_None;
			float new_value = (float&)it->second.data3;
			if( new_value != *value )
			{
				*value = new_value;
				return true;
			}
			return false;
		}
		else( float & ) it->second.data3 = *value; // I love this!

		return false;
	}

}


UILayer::UILayer()
{
	pWindowManager = CreateScoped<ui::WindowManager>( 1280, 720 );
}

static constexpr float fontSize = 8.0f;
static simple_timer gtimer;

void UILayer::OnCreate()
{
	g_pRenderer2D = Renderer2D::Create( GetApp()->GetGraphics() );
	FontManager::SetFont( "$ui", "../resources/Fonts/NunitoSans-Regular.ttf", fontSize );
	g_pRenderer2D->SelectFont( FontManager::GetFont( "$ui" ) );

	gtimer.reset();
}

	void UILayer::RemoveInActive()
	{
		for( auto && [k, w] : g_WindowContext )
		{
			if( w.uid == ui::null_window_uid ) continue;

			auto window = (DogeWindow *)pWindowManager->findWindow( w.uid );

			for( auto && [wk, widget] : w.widgets )
			{
				if( widget.uid != ui::null_window_uid && widget.use_count-- <= 0 )
				{
					using namespace ui;
					auto it = std::find_if( window->widgets.begin(), window->widgets.end(), [widget] ( ui::Widget * w ) { return ( w->getuid() == widget.uid ); } );
					ui::Window * wnd = * it;
					if( it != window->widgets.end() )
					{
						wnd->Release();
						delete wnd;
						widget.uid = ui::null_window_uid;
					}
				}
			}
		}
		
		auto & w = g_FallbackWindow;

		if( w.uid == ui::null_window_uid ) return;

		auto window = (DogeWindow *)pWindowManager->findWindow( w.uid );

		for( auto && [wk, widget] : w.widgets )
		{
			if( widget.uid != ui::null_window_uid && widget.use_count-- <= 0 )
			{
				using namespace ui;
				auto it = std::find_if( window->widgets.begin(), window->widgets.end(), [widget] ( ui::Widget * w ) { return ( w->getuid() == widget.uid ); } );
				ui::Window * wnd = *it;
				if( it != window->widgets.end() )
				{
					wnd->Release();
					delete wnd;
					widget.uid = ui::null_window_uid;
				}
			}
		}
	}

	void UILayer::CreateActive()
	{
		for( auto && [k, w] : g_WindowContext )
		{
			DogeWindow * window = nullptr;
			if( w.uid == ui::null_window_uid )
			{
				window = new DogeWindow( w.name );
				pWindowManager->addWindow( window );
				w.uid = window->getuid();
			}
			else window = (DogeWindow *)pWindowManager->findWindow( w.uid );

			for( auto && [wk, widget] : w.widgets )
			{
				if( widget.uid == ui::null_window_uid && widget.use_count > 0 )
				{
					auto w = new Slider( widget.name, window->Rect.width(), int( fontSize + window->offsetY+19*window->widgets.size() ), &widget );
					window->addWidget( w );
					widget.uid = w->getuid();
				}
			}
		}

		auto & w = g_FallbackWindow;

		DogeWindow * window = nullptr;
		if( w.uid == ui::null_window_uid )
		{
			window = new DogeWindow( w.name );
			pWindowManager->addWindow( window );
			w.uid = window->getuid();
		}
		else window = (DogeWindow *)pWindowManager->findWindow( w.uid );

		for( auto && [wk, widget] : w.widgets )
		{
			if( widget.uid == ui::null_window_uid && widget.use_count > 0 )
			{
				auto w = new Slider( widget.name, window->Rect.width(), int( fontSize + window->offsetY + 19 * window->widgets.size() ), &widget );
				window->addWidget( w );
				widget.uid = w->getuid();
			}
		}
	}

void UILayer::OnUpdate()
{
	g_Mutex.lock();

	RemoveInActive();
	CreateActive();

	g_Mutex.unlock();

	pWindowManager->OnUpdate( gtimer.gets() );
	g_pRenderer2D->Render();
}
