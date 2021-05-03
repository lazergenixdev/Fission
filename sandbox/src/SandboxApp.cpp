
#if defined(DEBUG)
#define FISSION_DEBUG
#elif defined(RELEASE)
#define FISSION_RELEASE
#elif defined(DIST)
#define FISSION_DIST
#else
#error "Unknown Configuration"
#endif

#include <Fission/Platform/EntryPoint.h>
#include <Fission/Fission.h>
#include <Fission/Core/SurfaceMap.h>
#include <Fission/Core/UI/UI.h>
#include <Fission/Core/Sound.h>
#include <Fission/Core/Monitor.h>
using namespace Fission;

#include <shellapi.h>
#pragma comment(lib, "shell32")
#pragma comment(lib, "user32")

#include <fstream>
#include <random>

static vec2f res = { 1280,720 };

scoped<Renderer2D> renderer2d;

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
#include "lazerui.h"

class DogeWindow : public lazer::ui::DynamicWindow
{
public:
	DogeWindow( Renderer2D * pr2d ) : DynamicWindow( { 100, 212, 100, 258 } ), m_pr2d(pr2d) {
		id.resize( 64 );
		swprintf_s( &id[0], 64, L"Debug Window" );
		id.shrink_to_fit();
	}
	virtual lazer::ui::Result OnSetCursor( lazer::ui::SetCursorEventArgs & args ) override
	{
		if( DynamicWindow::OnSetCursor( args ) )
		{
			auto pos = lazer::ui::GetRectPos( Rect, lazer::ui::g_MousePosition, 8, 4, 8 );
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

	virtual void OnResize() override
	{
		static constexpr int min_w = 110;
		static constexpr int min_h = 90;
		switch( state & State_Sizing )
		{
		case State_SizingL: Rect.x.low = std::min( Rect.x.low, Rect.x.high - min_w ); break;
		case State_SizingR: Rect.x.high = std::max( Rect.x.high, Rect.x.low + min_w ); break;
		case State_SizingT: Rect.y.low = std::min( Rect.y.low, Rect.y.high - min_h ); break;
		case State_SizingB: Rect.y.high = std::max( Rect.y.high, Rect.y.low + min_h ); break;
		case ( State_SizingL | State_SizingT ): 
			Rect.x.low = std::min( Rect.x.low, Rect.x.high - min_w ), Rect.y.low = std::min( Rect.y.low, Rect.y.high - min_h );
			break;
		case ( State_SizingR | State_SizingT ): 
			Rect.x.high = std::max( Rect.x.high, Rect.x.low + min_w ), Rect.y.low = std::min( Rect.y.low, Rect.y.high - min_h );
			break;
		case ( State_SizingL | State_SizingB ): 
			Rect.x.low = std::min( Rect.x.low, Rect.x.high - min_w ), Rect.y.high = std::max( Rect.y.high, Rect.y.low + min_h );
			break;
		case ( State_SizingR | State_SizingB ): 
			Rect.x.high = std::max( Rect.x.high, Rect.x.low + min_w ), Rect.y.high = std::max( Rect.y.high, Rect.y.low + min_h );
			break;
		default:
			break;
		}
	}

	virtual void OnUpdate(float) override
	{
		m_pr2d->FillRect( (rectf)( Rect ), Colors::DogeHouse );
		if( parent->GetFocus() == this )
		m_pr2d->DrawRect( (rectf)( Rect ), Colors::Gray, 1.0f, StrokeStyle::Outside );

		m_pr2d->DrawString( id.c_str(), (vec2f)Rect.get_tl(), Colors::Gray );

		m_pr2d->PushTransform( mat3x2f::Translation( (vec2f)Rect.get_tl() ) );
		DynamicWindow::OnUpdate(0.0f);
		m_pr2d->PopTransform();
	}
private:
	Renderer2D * m_pr2d;
	std::wstring id;
};

class Button : public lazer::ui::Button
{
public:
	Button( const wchar_t * label, Renderer2D * pr2d, std::function<void()> f, vec2i pos = { 10, 30 } ) : 
		Rect( recti::from_tl( pos, 90, 20 ) ), m_pr2d( pr2d ), label( label ) , on_press(f)
	{}
	virtual bool isInside( lazer::ui::point pos ) override { return Rect[pos]; }
	virtual void OnPressed() override { on_press(); }

	virtual lazer::ui::Result OnSetCursor( lazer::ui::SetCursorEventArgs & args ) override
	{
		args.cursor = Cursor::Get( Cursor::Default_Hand );
		return lazer::ui::Handled;
	}
	virtual void OnUpdate(float) override
	{
		if( active )
		m_pr2d->FillRect( (rectf)Rect, color(0.2f, 0.2f, 0.2f)*1.4f );
		else if( parent->GetHover() == this )
		m_pr2d->FillRect( (rectf)Rect, color(0.2f, 0.2f, 0.2f)*1.2f );
		else
		m_pr2d->FillRect( (rectf)Rect, color( 0.2f, 0.2f, 0.2f ) );
		if( parent->GetFocus() == this )
		m_pr2d->DrawRect( (rectf)Rect, color(Colors::White,0.1f), 1.0f );

		vec2f size = (vec2f)Rect.size();
		auto tl = m_pr2d->CreateTextLayout( label.c_str() );

		m_pr2d->DrawString( label.c_str(), (size - vec2f( tl.width, tl.height ))*0.5f + (vec2f)Rect.get_tl(), Colors::Gray );
	}
private:
	recti Rect;
	Renderer2D * m_pr2d;
	std::wstring label;
	std::function<void()> on_press;
};

class UILayer : public ILayer
{
public:
	UILayer() : wm( (int)res.x, (int)res.y ) {}

	virtual EventResult OnMouseMove(MouseMoveEventArgs&args) override
	{
		lazer::ui::MouseMoveEventArgs m;
		m.pos = args.position;
		return (EventResult)wm.OnMouseMove( m );
	}
	virtual EventResult OnKeyDown( KeyDownEventArgs&args) override
	{
		lazer::ui::KeyDownEventArgs m;
		m.key = args.key;
		return (EventResult)wm.OnKeyDown( m );
	}
	virtual EventResult OnKeyUp( KeyUpEventArgs&args) override
	{
		lazer::ui::KeyUpEventArgs m;
		m.key = args.key;
		return (EventResult)wm.OnKeyUp( m );
	}
	virtual EventResult OnSetCursor( SetCursorEventArgs&args) override
	{
		lazer::ui::SetCursorEventArgs m;
		m.cursor = args.cursor;
		EventResult r = (EventResult)wm.OnSetCursor( m );
		args.cursor = m.cursor;
		return r;
	}

	virtual void OnCreate() override;
	virtual void OnUpdate() override
	{
		renderer2d->SelectFont( FontManager::GetFont( "$console" ) );
		wm.OnUpdate( 0.0f );
		renderer2d->Render();
	}

private:
	lazer::ui::WindowManager wm;
};

class LoadingScene : public Fission::Scene
{
public:
	LoadingScene()
	{
		PushLayer( new UILayer );
	}
};


void UILayer::OnCreate()
{

	auto wnd = new DogeWindow( renderer2d.get() );
	wnd->addWidget( new Button( L"Exit", renderer2d.get(), [] { Fission::Application::Get()->Exit(); } ) );
	wnd->addWidget( new Button( L"Nothing", renderer2d.get(), [] {}, { 10,60 } ) );
	wnd->addWidget( new Button( L"Credits", renderer2d.get(), [] { Fission::Application::Get()->PushScene( "unused", new LoadingScene ); }, { 10,90 } ) );
	wnd->addWidget( new Button( L"Back", renderer2d.get(), [] { Fission::Application::Get()->CloseScene(); }, { 10,120 } ) );
	wm.addWindow( wnd );
}

class SandboxApp : public Application
{
public:
	SandboxApp() : Application( { nullptr, L"sandbox (demo scene switching)" } ) {}

	virtual void OnCreate() override 
	{
		PushScene( "unused", new LoadingScene );

		renderer2d = Renderer2D::Create( GetGraphics() );
	}
};

Application * CreateApplication() {
	return new SandboxApp;
}