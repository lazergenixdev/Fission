#include "UILayer.h"
#include <Fission/Core/UI/UI.hh>
#include <Fission/Core/Application.hh>
#include <Fission/Base/Utility/Timer.h>

// Everything about this code is awful, please avert your eyes for your own well being. You have been warned.

using namespace Fission;
using namespace react;

static scoped<IFRenderer2D>			g_pRenderer2D;
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
	ui::Widget * widget = nullptr;

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
		ui::SetCursorEventArgs m;
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

static color g_ColorBackground = Colors::make_gray( 0.2f );

class DogeWindow : public ui::DynamicWindow
{
public:
	DogeWindow( std::string label ) : DynamicWindow( { 100, 212, 100, 258 } ), id(label) {}
	virtual ui::Result OnSetCursor( ui::SetCursorEventArgs & args ) override
	{
		if( DynamicWindow::OnSetCursor( args ) )
		{
			auto pos = ui::GetRectPos( Rect, ui::g_MousePosition, 8, 0, 8 );
			switch( pos )
			{
			case ui::rect_pos_left:			args.cursor = Cursor::Get( Cursor::Default_SizeX ); break;
			case ui::rect_pos_right:			args.cursor = Cursor::Get( Cursor::Default_SizeX ); break;
			case ui::rect_pos_top:			args.cursor = Cursor::Get( Cursor::Default_SizeY ); break;
			case ui::rect_pos_bottom:		args.cursor = Cursor::Get( Cursor::Default_SizeY ); break;
			case ui::rect_pos_top_left:		args.cursor = Cursor::Get( Cursor::Default_SizeTLBR ); break;
			case ui::rect_pos_top_right:		args.cursor = Cursor::Get( Cursor::Default_SizeBLTR ); break;
			case ui::rect_pos_bottom_left:	args.cursor = Cursor::Get( Cursor::Default_SizeBLTR ); break;
			case ui::rect_pos_bottom_right:	args.cursor = Cursor::Get( Cursor::Default_SizeTLBR ); break;
			default: break;
			}
		}
		return ui::Handled;
	}

	virtual void OnUpdate( float dt ) override
	{
		base::rectf rect = (base::rectf)Rect;

		auto tl = g_pRenderer2D->CreateTextLayout( id.c_str() );

		g_pRenderer2D->FillRect( rect, g_ColorBackground );
		g_pRenderer2D->FillRect( base::rectf::from_topleft(rect.left()+60.0f,rect.top(),rect.width()-60.0f,tl.height), Colors::make_gray( 0.1f ) );
		
		g_pRenderer2D->DrawString( id.c_str(), rect.topLeft()+base::vector2f((60.0f-tl.width)*0.5f,0.0f), Colors::Snow );

		g_pRenderer2D->DrawRect( rect, Colors::Black, 2.0f, StrokeStyle::Outside );

		g_pRenderer2D->PushTransform( base::matrix2x3f::Translation( rect.left(), rect.top() ) );
		DynamicWindow::OnUpdate( dt );
		g_pRenderer2D->PopTransform();
	}
public:
	float offsetY = 14.0f;
private:
	std::string id;
};

class Slider : public ui::Slider
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

	virtual ui::Result OnMouseMove( ui::MouseMoveEventArgs & args ) override
	{
		if( dragging )
		{
			ui::point offset = args.pos - startDrag;
			drag_number( startNumber + float(offset.x/2)*0.1f );
			return ui::Handled;
		}

		ui::rect dragRect = { Rect.left(), Rect.x.average(), Rect.top(), Rect.bottom() };

		int center = Rect.x.average();
		int pos = ui::GetMousePosition().x - parent->Rect.left();

		inTextBox = ( pos > center );

		return ui::Handled;
	}
	virtual ui::Result OnMouseLeave() override
	{
		inTextBox = false;
		return ui::Handled;
	}

	virtual ui::Result OnKeyDown( ui::KeyDownEventArgs & args ) override
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
		return ui::Handled;
	}
	virtual ui::Result OnKeyUp( ui::KeyUpEventArgs & args ) override
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
		return ui::Handled;
	}

	virtual ui::Result OnTextInput( ui::TextInputEventArgs & args ) override
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
		return ui::Handled;
	}

	virtual ui::Result OnSetCursor( ui::SetCursorEventArgs & args ) override
	{
		ui::rect dragRect = { Rect.left(), Rect.x.average(), Rect.top(), Rect.bottom() };

		int center = Rect.x.average();
		int pos = ui::GetMousePosition().x - parent->Rect.left();

		if( pos < center )
			args.cursor = Cursor::Get( Cursor::Default_Arrow );
		else
			args.cursor = Cursor::Get( Cursor::Default_TextInput );


		return ui::Pass;
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
		base::rectf rect = (base::rectf)Rect;

		g_pRenderer2D->DrawString( label.c_str(), rect.topLeft(), this == parent->GetHover() ? Colors::White : Colors::make_gray( 0.7f ) );

		base::rectf numberbox = { rect.x.average(), rect.right(), rect.top(), rect.bottom() };

		g_pRenderer2D->FillRect( numberbox, Colors::make_gray( 0.15f ) );

		if( inTextBox ) selectAlpha += ( 0.35f - selectAlpha ) * dt * 5.0f;
		else			selectAlpha += ( 0.00f - selectAlpha ) * dt * 5.0f;

		if( selectAlpha > 0.0f )
		g_pRenderer2D->DrawRect( numberbox, color(1.0f, 1.0f, 1.0f, selectAlpha), 1.0f, StrokeStyle::Outside );

		auto tl = g_pRenderer2D->DrawString( numberText.c_str(), base::vector2f::from(numberbox.topLeft()), Colors::White );

		if( this == parent->GetFocus() && not dragging )
			g_pRenderer2D->FillRect(
				base::rectf::from_center( (float)(int)(numberbox.left() + tl.width) + 0.5f, numberbox.y.average(), 1.0f, tl.height-4.0f ),
				Colors::White );
	}
public:
	std::string numberText;
private:
	std::string label;
	ui::rect Rect;
	bool inTextBox = false;
	float selectAlpha = 0.0f;

	bool dragging = false;
	ui::point startDrag = {};
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
		else
		{
			if( (float &)it->second.data3 != *value && it->second.widget )
			{
				static_cast<Slider *>( it->second.widget )->numberText = std::to_string( *value );
				(float &)it->second.data3 = *value;
			}
		}

		return false;
	}

}


UILayer::UILayer()
{
	pWindowManager = make_scoped<ui::WindowManager>( 1280, 720 );
}

static constexpr float fontSize = 8.0f;
static simple_timer gtimer;

void UILayer::OnCreate()
{
//	g_pRenderer2D = Renderer2D::Create( GetApp()->GetGraphics() );
//	FontManager::SetFont( "$ui", "../resources/Fonts/NunitoSans-Regular.ttf", fontSize );
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
				if( widget.widget && widget.use_count-- <= 0 )
				{
					widget.widget->Release();
					delete widget.widget;
					widget.widget = nullptr;
				}
			}
		}
		
		auto & w = g_FallbackWindow;

		if( w.uid == ui::null_window_uid ) return;

		auto window = (DogeWindow *)pWindowManager->findWindow( w.uid );

		for( auto && [wk, widget] : w.widgets )
		{
			if( widget.widget && widget.use_count-- <= 0 )
			{
				widget.widget->Release();
				delete widget.widget;
				widget.widget = nullptr;
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
				if( widget.widget == nullptr && widget.use_count > 0 )
				{
					auto w = new Slider( widget.name, window->Rect.width(), int( fontSize + window->offsetY+19*window->widgets.size() ), &widget );
					window->addWidget( w );
					widget.widget = w;
				}
			}
		}

		auto & w = g_FallbackWindow;
		if( w.use_count )
		{
			--w.use_count;
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
				if( widget.widget == nullptr && widget.use_count > 0 )
				{
					auto w = new Slider( widget.name, window->Rect.width(), int( fontSize + window->offsetY + 19 * window->widgets.size() ), &widget );
					window->addWidget( w );
					widget.widget = w;
				}
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
