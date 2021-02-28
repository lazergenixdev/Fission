#include "UILayer.h"
#include "LazerEngine/Core/Application.h"
#include <lazer/math.h>

#define _lazer_key_type lazer::Keys::Key
#define _lazer_key_left_mouse_ lazer::Keys::Mouse_Left
#define _lazer_key_right_mouse_ lazer::Keys::Mouse_Right
#define _lazer_char_type wchar_t
#define _lazer_cursor_type lazer::Cursor *
#include "lazerui.h"

#include <LazerEngine/Core/UI/UI.h>

#include "LazerEngine/Core/Console.h"

namespace NunitoSemiBoldTTF {
#include "Static Fonts/Nunito-SemiBold.inl"
}

using namespace lazer;

static std::unique_ptr<ui::WindowManager> g_pWindowManager;

static std::unique_ptr<Renderer2D> g_pRenderer2D;


class SandboxWindow : public lazer::ui::DynamicWindow
{
public:
	SandboxWindow( const char * name, int x, int y, int width, int height, lazer::color col ) :
		lazer::ui::DynamicWindow( { x, x + width, y, y + height } ),
		window_color( col ),
		title( utf8_to_wstring( name ) )
	{}

	virtual void OnUpdate( float dt ) override
	{
		using namespace lazer;

		g_pRenderer2D->FillRect( (rectf)Rect, Colors::make_color( window_color, 0.1f, 0.94f ) );

		auto tl = g_pRenderer2D->CreateTextLayout( title.c_str() );
		auto titleRc = (rectf)Rect;
		titleRc.y.high = titleRc.y.low + tl.height;
		if( parent->GetFocus() == this )
			g_pRenderer2D->FillRect( (rectf)titleRc, Colors::make_color( window_color, 0.5f ) * coloru( 255, 255, 255, 100 ) );
		else
			g_pRenderer2D->FillRect( (rectf)titleRc, coloru( 0, 0, 0, 100 ) );

		color c0 = Colors::make_color( window_color, 0.7f );
		color c1 = window_color;
		g_pRenderer2D->DrawRect( (rectf)Rect, ( ( parent->GetHover() == this ) ? c1 : c0 ), 2.0f );

		g_pRenderer2D->DrawString( title.c_str(), (vec2f)Rect.get_tl() + vec2f( 2.0f, 2.0f ), Colors::White );

		starty = tl.height;

		g_pRenderer2D->PushTransform( mat3x2f::Translation( (vec2f)Rect.get_tl() ) );
		DynamicWindow::OnUpdate( dt );
		g_pRenderer2D->PopTransform();
	}

	float starty;
	float padding = 5.0f;
private:
	std::wstring title;
	lazer::color window_color;
};

class SandboxTextEdit : public lazer::ui::TextEdit
{
public:
	SandboxTextEdit( lazer::vec2i pos, lazer::vec2i size ) :
		pos( pos ), size( size )
	{}

	virtual void OnFinishEdit() override {
	//	ShellExecuteA( NULL, "open", lazer::wstring_to_utf8( searchText + text ).c_str(), NULL, NULL, 5 );
	}

	virtual bool isInside( lazer::vec2i pos ) override {
		return lazer::recti::from_tl( parent->Rect.get_tl() + this->pos, size )[pos];
	}

	virtual lazer::ui::Result OnKeyDown( lazer::ui::KeyDownEventArgs & args ) override {
		switch( args.key )
		{
		case lazer::Keys::Mouse_Left:
		{
			if( !text.empty() )
			{
				//auto p = (lazer::vec2f)( lazer::ui::GetMousePosition() - parent->Rect.get_tl() - pos - lazer::vec2(2,0) );
				//if( auto i = tl.test_intersection( p ) )
				//{
				//	curPos = i.value();
				//	if( curPos < tl.ch_divisions.size() )
				//		cursorOffset = tl.ch_divisions[curPos].start;
				//	else cursorOffset = tl.width;
				//}
				//else { curPos = (int)text.size(); cursorOffset = tl.width; }
			}
			break;
		}
		default:break;
		}
		return Widget::OnKeyDown( args );
	}

	lazer::ui::Result OnSetCursor( lazer::ui::SetCursorEventArgs & args ) override {
		args.cursor = lazer::Cursor::Get( lazer::Cursor::Default_TextInput );
		return lazer::ui::Handled;
	}

	virtual lazer::ui::Result OnTextInput( lazer::ui::TextInputEventArgs & args ) override {
		switch( args.ch )
		{
		case '\b':
			if( !text.empty() ) {
				text.pop_back();
			}
			break;
		case '\r':
			parent->SetFocus( nullptr );
			OnFinishEdit();
			break;
		default:
		{
			if( text.size() < 25u ) {
				text.push_back( args.ch );
			}
			break;
		}
		}
		return lazer::ui::Handled;
	}

	virtual void OnUpdate( float dt ) override
	{
		auto owner = static_cast<SandboxWindow *>( parent );
		pos.y = (int)( owner->starty + owner->padding );
		pos.x = (int)owner->padding;
		owner->starty += ( (float)size.y + owner->padding );

		color c0 = { 1.0f, 1.0f, 1.0f, 0.1f };
		color c1 = { 1.0f, 1.0f, 1.0f, 0.2f };

		auto rect = rectf::from_tl( (vec2f)pos, (vec2f)size );

		g_pRenderer2D->FillRect( rect, parent->GetHover() == this ? c1 : c0 );
		g_pRenderer2D->DrawRect( rect, parent->GetFocus() == this ? c1 * 3.0f : c0 * 2.0f, 1.0f );

		if( text.empty() && parent->GetFocus() != this )
			g_pRenderer2D->DrawString( L"Search . . .", rect.get_tl() + vec2( 2.0f, 0.0f ), coloru( 255, 255, 255, 100 ) );
		else
		{
			auto tl = g_pRenderer2D->DrawString( text.c_str(), rect.get_tl() + vec2( 2.0f, 0.0f ), Colors::White );
			if( parent->GetFocus() == this )
			{
				vec2f cur_size = { 1.0f, size.y * 0.8f };
				g_pRenderer2D->FillRect( rectf::from_tl( rect.get_tl() + vec2f{ 2.0f + tl.width, size.y * 0.1f }, cur_size ), Colors::White );
			}
		}

	}
private:
	lazer::vec2i pos, size;
	std::wstring text;
};

class SandboxSlider : public ui::Slider
{
public:
	SandboxSlider( const char * name, float * pvar, float min, float max, vec2i size ) :
		title( utf8_to_wstring( name ) ), pValue( pvar ), min( min ), max( max ), size( size )
	{}

	virtual bool isInside( vec2i pos ) override {
		return recti::from_tl( parent->Rect.get_tl() + this->pos, size )[pos];
	}

	virtual void UpdateSlidePosition( vec2i mouse ) override {
		auto rect = rectf::from_tl( (vec2f)pos, (vec2f)size );
		float padding = slide_size.x * 0.5f - rect.height() * 0.2f;
		vec2f p = vec2f( mouse - parent->Rect.get_tl() );
		auto temp = map( p.x, (float)pos.x + padding, (float)( pos.x + size.x ) - padding, min, max );
		*pValue = std::clamp( temp, min, max );
	}

	virtual void OnUpdate( float dt ) override
	{
		color c0 = { 1.0f, 1.0f, 1.0f, 0.1f };
		color c1 = { 1.0f, 1.0f, 1.0f, 0.2f };

		auto owner = static_cast<SandboxWindow *>( parent );
		pos.y = (int)( owner->starty + owner->padding );
		pos.x = (int)owner->padding;
		owner->starty += ( (float)size.y + owner->padding );

		auto rect = rectf::from_tl( (vec2f)pos, (vec2f)size );

		if( active )
			g_pRenderer2D->FillRect( rect, color( 0.5f, 0.8f, 1.0f, 0.4f ) );
		else
			g_pRenderer2D->FillRect( rect, parent->GetHover() == this ? c1 : c0 );

		if( parent->GetFocus() == this )
			g_pRenderer2D->DrawRect( rect, c0 * 2.0f, 1.0f );

		float effective_width = rect.width() - slide_size.x - rect.height() * 0.4f;

		float slide_pos = map( *pValue, min, max, 0.0f, 1.0f );


		wchar_t buffer[20];
		swprintf_s( buffer, L"%.4f", *pValue );
		auto tl = g_pRenderer2D->CreateTextLayout( buffer );
		g_pRenderer2D->DrawString( buffer,
			vec2(
				rect.get_l() + ( rect.width() - tl.width ) * 0.5f,
				rect.get_t() + ( rect.height() - tl.height ) * 0.5f ),
			coloru( 255, 255, 255, 150 )
		);
		tl = g_pRenderer2D->CreateTextLayout( title.c_str() );
		g_pRenderer2D->DrawString( title.c_str(), vec2( rect.get_r() + 4.0f, rect.y.get_average() - tl.height * 0.5f ), Colors::White );

		g_pRenderer2D->FillRect( rectf::from_center( { rect.get_l() + slide_size.x * 0.5f + slide_pos * effective_width + rect.height() * 0.2f, rect.y.get_average() }, { slide_size } ), c0 * 3.0f );

	}
private:
	std::wstring title;
	vec2i pos, size;
	vec2f slide_size = { 6.0f, (float)size.y * 0.60f };
	float min, max;
	float * pValue;
};

class SandboxButton : public ui::Button
{
public:
	SandboxButton( const char * name, std::function<void()> action ) :
		title( utf8_to_wstring( name ) ), action( action )
	{}

	virtual void OnPressed() override {
		action();
	}

	virtual bool isInside( vec2i pos ) override {
		return recti::from_tl( parent->Rect.get_tl() + this->pos, size )[pos];
	}

	ui::Result OnSetCursor( ui::SetCursorEventArgs & args ) override {
		args.cursor = Cursor::Get( Cursor::Default_Hand );
		return ui::Handled;
	}

	virtual void OnUpdate( float dt ) override
	{
		color c0 = { 1.0f, 1.0f, 1.0f, 0.1f };
		color c1 = { 1.0f, 1.0f, 1.0f, 0.2f };
		auto tl = g_pRenderer2D->CreateTextLayout( title.c_str() );

		auto owner = static_cast<SandboxWindow *>( parent );
		pos.y = (int)( owner->starty + owner->padding );
		pos.x = (int)owner->padding;
		owner->starty += ( (float)size.y + owner->padding );
		size.x = (int)( tl.width + owner->padding * 2.0f );
		size.y = (int)( tl.height );

		auto rect = rectf::from_tl( (vec2f)pos, (vec2f)size );

		if( active && parent->GetHover() == this )
			g_pRenderer2D->FillRect( rect, color( 1.0f, 1.0f, 1.0f, 0.5f ) );
		else
			g_pRenderer2D->FillRect( rect, parent->GetHover() == this ? c1 : c0 );

		g_pRenderer2D->DrawRect( rect, c0 * 2.0f, parent->GetFocus() == this ? 1.5f : 0.5f );

		g_pRenderer2D->DrawString( title.c_str(),
			vec2( 
				rect.get_l() + ( rect.width() - tl.width ) * 0.5f, 
				rect.get_t() + ( rect.height() - tl.height ) * 0.5f ),
			coloru( 255, 255, 255, 200 ) 
		);
	}
private:
	std::wstring title;
	vec2i pos, size;
	std::function<void()> action;
};

UILayer::UILayer()
{
	g_pWindowManager = std::make_unique<ui::WindowManager>( 1280, 720 );
}

void UILayer::OnCreate()
{
	g_pRenderer2D = Renderer2D::Create( GetApp()->GetGraphics() );
	FontManager::SetFont( "$ui", NunitoSemiBoldTTF::data, NunitoSemiBoldTTF::size, 10.0f );
	g_pRenderer2D->SelectFont( FontManager::GetFont( "$ui" ) );
}

void UILayer::OnUpdate()
{
	g_pWindowManager->OnUpdate( 0.0f );
	g_pRenderer2D->Render();
}

EventResult UILayer::OnKeyDown( KeyDownEventArgs & args )
{
	ui::KeyDownEventArgs _ui_args{ args.key };
	return (EventResult)g_pWindowManager->OnKeyDown( _ui_args );
}

EventResult UILayer::OnKeyUp( KeyUpEventArgs & args )
{
	ui::KeyUpEventArgs _ui_args{ args.key };
	return (EventResult)g_pWindowManager->OnKeyUp( _ui_args );
}

EventResult UILayer::OnTextInput( TextInputEventArgs & args )
{
	ui::TextInputEventArgs _ui_args{ args.character };
	return (EventResult)g_pWindowManager->OnTextInput( _ui_args );
}

EventResult UILayer::OnMouseMove( MouseMoveEventArgs & args )
{
	ui::MouseMoveEventArgs _ui_args{ args.position };
	return (EventResult)g_pWindowManager->OnMouseMove( _ui_args );
}

EventResult UILayer::OnMouseLeave( MouseLeaveEventArgs & args )
{
	return (EventResult)g_pWindowManager->OnMouseLeave();
}

EventResult UILayer::OnSetCursor( SetCursorEventArgs & args )
{
	ui::SetCursorEventArgs _ui_args{ args.cursor };
	auto r = g_pWindowManager->OnSetCursor( _ui_args );
	args.cursor = _ui_args.cursor;
	return (EventResult)r;
}


/* UI */

UI::Key::Key() : context( nullptr ) {}

UI::Key::Key(void * ptr) : context( ptr ) {}

UI::WindowKey & UI::WindowKey::operator=( UI::WindowKey && src ) {
	context = src.context;
	src.context = nullptr;
	return *this;
}
UI::SliderKey & UI::SliderKey::operator=( UI::SliderKey && src ) {
	context = src.context;
	src.context = nullptr;
	return *this;
}
UI::ButtonKey & UI::ButtonKey::operator=( UI::ButtonKey && src ) {
	context = src.context;
	src.context = nullptr;
	return *this;
}

UI::Key::~Key() {
	if( !context ) return;
	static_cast<ui::Window *>( context )->Release();
	delete context;
	context = nullptr;
}

UI::WindowKey::WindowKey( void * ptr ) : Key( ptr ) {}

UI::ButtonKey::ButtonKey( void * ptr ) : Key( ptr ) {}

UI::SliderKey::SliderKey( void * ptr ) : Key( ptr ) {}

UI::WindowKey::WindowKey( const WindowKey & src ) : Key( src.context ) { *(void**)&src.context = nullptr; }

UI::ButtonKey::ButtonKey( const ButtonKey & src ) : Key( src.context ) { *(void**)&src.context = nullptr; }

UI::SliderKey::SliderKey( const SliderKey & src ) : Key( src.context ) { *(void**)&src.context = nullptr; }

UI::WindowKey UI::PushWindow( const char * name, int x, int y, int width, int height ) {
	auto w = new SandboxWindow( name, x, y, width, height, Colors::DodgerBlue );
	g_pWindowManager->addWindow( w );
	return UI::WindowKey( (void *)w );
}

UI::ButtonKey UI::WindowKey::PushButton( const char * name, std::function<void()> action ) {
	if( !context ) return { nullptr };
	auto w = new SandboxButton( name, action );
	static_cast<ui::DynamicWindow *>( context )->addWidget( w );
	return w;
}

UI::SliderKey UI::WindowKey::PushSliderFloat( const char * name, float * pvar, float min, float max, const char * fmt ) {
	if( !context ) return { nullptr };
	auto w = new SandboxSlider( name, pvar, min, max, { 150, 16 } );
	static_cast<ui::DynamicWindow *>( context )->addWidget( w );
	return w;
}

