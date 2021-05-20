#include "UILayer.h"
#include <Fission/Core/UI/UI.h>
#include <Fission/Core/Application.h>

// Everything about this code is awful, please avert your eyes for your own well being. You have been warned.

using namespace Fission;

#define _lazer_has_vector
#define _lazer_has_point
#define _lazer_has_rect
namespace lazer::ui {
	template <typename T> using vector = std::vector<T>;
	using point = vec2i;
	using rect = recti;
}
#define _lazer_char_type wchar_t
#define _lazer_key_type Keys::Key
#define _lazer_key_left_mouse_ Keys::Mouse_Left
#define _lazer_key_right_mouse_ Keys::Mouse_Right
#define _lazer_cursor_type Cursor *
#include <Fission/reactui.h>

static scoped<Renderer2D>			g_pRenderer2D;
static scoped<ui::WindowManager>	g_pWindowManager;
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
#define SUBTYPE /* This is a full type (IDE helper) */

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


	// For these types `data3` is used to store the numeric value
	SUBTYPE type_input_number = 0x0,

	TYPE input_float		= type_input_number | 0x1,
	TYPE input_int			= type_input_number | 0x2,
	TYPE input_int64		= type_input_number | 0x3,
	TYPE input_float64		= type_input_number | 0x4,


	// Not implemented
	SUBTYPE type_input_string = 0x4000000,

	TYPE input_string		= type_input_string | 0x0,
	TYPE input_ml_string	= type_input_string | 0x1,

#undef SUBTYPE
#undef TYPE
};

struct WidgetInfo
{
	std::string name;
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
	ui::window_uid uid = ui::null_window_uid;
	std::unordered_map<std::string,WidgetInfo> widgets;
};

struct FallbackWindowInfo : public WindowInfo
{
	bool used = false;
};

static std::unordered_map<std::string, WindowInfo> g_WindowContext;
static WindowInfo * g_pActiveWindow;
static FallbackWindowInfo g_pFallbackWindow = {"Debug"};


namespace Fission
{
	EventResult UILayer::OnMouseMove( MouseMoveEventArgs & args )
	{
		ui::MouseMoveEventArgs m;
		m.pos = args.position;
		std::scoped_lock lock( g_Mutex );
		return (EventResult)g_pWindowManager->OnMouseMove( m );
	}
	EventResult UILayer::OnKeyDown( KeyDownEventArgs & args )
	{
		ui::KeyDownEventArgs m;
		m.key = args.key;
		std::scoped_lock lock( g_Mutex );
		return (EventResult)g_pWindowManager->OnKeyDown( m );
	}
	EventResult UILayer::OnKeyUp( KeyUpEventArgs & args )
	{
		ui::KeyUpEventArgs m;
		m.key = args.key;
		std::scoped_lock lock( g_Mutex );
		return (EventResult)g_pWindowManager->OnKeyUp( m );
	}
	EventResult UILayer::OnTextInput( TextInputEventArgs & args )
	{
		ui::TextInputEventArgs m;
		m.ch = args.character;
		std::scoped_lock lock( g_Mutex );
		return (EventResult)g_pWindowManager->OnTextInput( m );
	}
	EventResult UILayer::OnSetCursor( SetCursorEventArgs & args )
	{
		lazer::ui::SetCursorEventArgs m;
		m.cursor = args.cursor;
		std::scoped_lock lock( g_Mutex );
		EventResult r = (EventResult)g_pWindowManager->OnSetCursor( m );
		args.cursor = m.cursor;
		return r;
	}
	EventResult UILayer::OnMouseLeave( MouseLeaveEventArgs & args )
	{
		std::scoped_lock lock( g_Mutex );
		return (EventResult)g_pWindowManager->OnMouseLeave();
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

	DynamicWindow * addWidget(ui::Widget* widget)
	{
		offsetY += 18.0f; // this is bad
		return DynamicWindow::addWidget( widget );
	}

	virtual void OnUpdate( float ) override
	{
		rectf rect = (rectf)Rect;

		auto tl = g_pRenderer2D->CreateTextLayout( id.c_str() );

		g_pRenderer2D->FillRect( rect, g_ColorBackground );
		g_pRenderer2D->FillRect( rectf::from_tl({rect.get_l()+60.0f,rect.get_t()},{rect.width()-60.0f,tl.height}), Colors::Gray_90_Percent );
		
		g_pRenderer2D->DrawString( id.c_str(), rect.get_tl()+vec2f((60.0f-tl.width)*0.5f,0.0f), Colors::Snow );

		g_pRenderer2D->DrawRect( rect, Colors::Black, 2.0f, StrokeStyle::Outside );

		g_pRenderer2D->PushTransform( mat3x2f::Translation( rect.get_tl() ) );
		DynamicWindow::OnUpdate( 0.0f );
		g_pRenderer2D->PopTransform();
	}
public:
	float offsetY = 5.0f;
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
			args.cursor = Cursor::Get( Cursor::Default_SizeX );
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

	virtual void OnUpdate( float ) override
	{
		rectf rect = (rectf)Rect;

		g_pRenderer2D->DrawString( label.c_str(), rect.get_tl(), this == parent->GetHover() ? Colors::White : Colors::Gray_30_Percent );

		rectf numberbox = { rect.x.get_average(), rect.get_r(), rect.get_t(), rect.get_b() };

		g_pRenderer2D->FillRect( numberbox, Colors::Gray_85_Percent );

		if( inTextBox )
		g_pRenderer2D->DrawRect( numberbox, color(1.0f, 1.0f, 1.0f, 0.35f), 1.0f, StrokeStyle::Outside );

		auto tl = g_pRenderer2D->DrawString( numberText.c_str(), numberbox.get_tl(), Colors::White );

		if( this == parent->GetFocus() )
			g_pRenderer2D->FillRect(
				rectf::from_center( vec2f( numberbox.get_l() + tl.width + 0.5f, numberbox.y.get_average() ), 1.0f, tl.height-4.0f ),
				Colors::White );
	}
private:
	std::string label;
	std::string numberText;
	recti Rect;
	bool inTextBox = false;

	WidgetInfo * pWidget;
};


namespace Fission::UI {

	inline WindowInfo * GetActiveWindow() { if( g_pActiveWindow ) return g_pActiveWindow; g_pFallbackWindow.used = true; return &g_pFallbackWindow; }

	inline void Reset() { if( g_pActiveWindow ) g_pActiveWindow = nullptr; }


	bool Debug::Window( const char * label )
	{
		if( !label ) { g_pActiveWindow= &g_pFallbackWindow; g_pFallbackWindow.used = true; return true; }

		std::string sLabel = label;
		auto it = g_WindowContext.find( sLabel );
		if( it == g_WindowContext.end() )
		{
			g_WindowContext.emplace( sLabel, sLabel );
			g_pActiveWindow = &g_WindowContext[sLabel];
			return true;
		}

		g_pActiveWindow = &it->second;
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
		if( it == window->widgets.end() )
		{
			WidgetInfo widget;
			widget.name = sLabel;
			widget.type = WidgetType::input_float;
			(float&)widget.data3 = *value; // don't store the pointer, we don't own this value!
			window->widgets.emplace( sLabel, widget );
			return false;
		}

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

	//bool Debug::InputInt( const char * label, int * value )
	//{
	//	auto window = GetActiveWindow();

	//	std::string sLabel = label;
	//	auto it = window->widgets.find( sLabel );
	//	if( it == window->widgets.end() )
	//	{
	//		WidgetInfo widget;
	//		widget.name = sLabel;
	//		widget.type = WidgetType::input_int;
	//		(int&)widget.data3 = *value; // don't store the pointer, we don't own this value!
	//		window->widgets.emplace( sLabel, widget );
	//		return false;
	//	}

	//	int new_value = (int&)it->second.data3;
	//	if( new_value != *value )
	//	{
	//		*value = new_value;
	//		return false;
	//	}

	//	return false;
	//}

}


UILayer::UILayer()
{
	g_pWindowManager = std::make_unique<ui::WindowManager>( 1280, 720 );
}

static constexpr float fontSize = 8.0f;

void UILayer::OnCreate()
{
	g_pRenderer2D = Renderer2D::Create( GetApp()->GetGraphics() );
	FontManager::SetFont( "$ui", "../resources/Fonts/NunitoSans-Regular.ttf", fontSize );
	g_pRenderer2D->SelectFont( FontManager::GetFont( "$ui" ) );
}

void UILayer::OnUpdate()
{
	g_Mutex.lock();
	for( auto && [k,w] : g_WindowContext )
	{
		DogeWindow * window = nullptr;
		if( w.uid == ui::null_window_uid )
		{
			window = new DogeWindow( w.name );
			g_pWindowManager->addWindow( window );
			w.uid = window->getuid();
		}
		else window = (DogeWindow *)g_pWindowManager->findWindow( w.uid );

		for( auto && [wk, widget] : w.widgets )
		{
			if( widget.uid == ui::null_window_uid )
			{
				auto w = new Slider( widget.name, window->Rect.width(), int( fontSize + window->offsetY ), &widget );
				window->addWidget( w );
				widget.uid = w->getuid();
			}
		}
		
	}
	if( g_pFallbackWindow.used )
	{
		// Give Fallback window to the Window Manager if it does not have it already
		DogeWindow * window = nullptr;
		if( g_pFallbackWindow.uid != ui::null_window_uid )
			window = (DogeWindow *)g_pWindowManager->findWindow( g_pFallbackWindow.uid );
		else 
		{
			window = new DogeWindow( g_pFallbackWindow.name );
			g_pWindowManager->addWindow( window );
			g_pFallbackWindow.uid = window->getuid();
		}

		for( auto && [wk, widget] : g_pFallbackWindow.widgets )
		{
			if( widget.uid == ui::null_window_uid )
			{
				auto w = new Slider( widget.name, window->Rect.width(), int( fontSize + window->offsetY ), &widget );
				window->addWidget( w );
				widget.uid = w->getuid();
			}
		}
	}
	g_Mutex.unlock();

	g_pWindowManager->OnUpdate(0.0f);
	g_pRenderer2D->Render();
}
