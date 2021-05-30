#include <Fission/Platform/EntryPoint.h>
#include <Fission/Fission.h>
#include <Fission/Core/SurfaceMap.h>
#include <Fission/Core/UI/UI.h>
#include <Fission/Core/Sound.h>
#include <Fission/Core/Graphics/Bindable.h>
using namespace Fission;

#include <numbers>

static vec2f res = { 1280,720 };

scoped<Renderer2D> renderer2d;
simple_timer gtimer;

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
#include "Fission/reactui.h"

class Button : public lazer::ui::Button
{
public:
	Button( const wchar_t * label, std::function<void()> f, vec2i pos = { 10, 30 } ) : 
		Rect( recti::from_tl( pos, 90, 20 ) ), label( label ) , on_press(f)
	{}
	virtual bool isInside( lazer::ui::point pos ) override { return Rect[pos]; }
	virtual void OnPressed() override { on_press(); }

	virtual lazer::ui::Result OnSetCursor( lazer::ui::SetCursorEventArgs & args ) override
	{
		args.cursor = Cursor::Get( Cursor::Default_Hand );
		return lazer::ui::Handled;
	}
	virtual void OnUpdate(float dt) override
	{
		// Easing based on the state of the button
		if( active )
			x += ( 0.6f - x ) * 20.0f * dt;
		else if( parent->GetHover() == this )
			x += ( 1.0f - x ) * 10.0f * dt;
		else
			x += ( 0.3f - x ) * 10.0f * dt;

		renderer2d->DrawRect( ((rectf)Rect).get_expanded(map(x,0.3f,1.0f,0.0f,4.0f)), color( c, x ), 1.5f );

		// center text inside button by getting the text boundries
		vec2f size = (vec2f)Rect.size();
		auto tl = renderer2d->CreateTextLayout( label.c_str() );
		renderer2d->DrawString( label.c_str(), (size - vec2f( tl.width, tl.height ))*0.5f + (vec2f)Rect.get_tl(), color(c,x*(1.0f/1.2f)) );
	}
private:
	recti Rect;
	float x = 0.3f;
	std::wstring label;
	color c = Colors::White;
	std::function<void()> on_press;
};

class CreditLayer : public ILayer
{
public:
	virtual EventResult OnKeyUp( KeyUpEventArgs & args ) override
	{
		if( args.key == Keys::Mouse_Left )
		{
		}
		return EventResult::Pass;
	}
	virtual EventResult OnKeyDown( KeyDownEventArgs&args) override
	{
		if( args.key == Keys::Escape )
		{
			Application::Get()->CloseScene();
			return EventResult::Handled;
		}
		return EventResult::Pass;
	}

	virtual void OnCreate() override {}
	virtual void OnUpdate() override
	{
		renderer2d->SelectFont( FontManager::GetFont( "$console" ) );
		renderer2d->DrawString( L"Lead Developer: Lazergenix", { 100.0f,50.0f }, Colors::White );
		renderer2d->DrawString( L"Contributing Developers: none :(", { 100.0f,65.0f }, Colors::White );

		renderer2d->SelectFont( FontManager::GetFont( "credit" ) );
		renderer2d->DrawString( L"External Software Libraries:", { 100.0f,120.0f }, Colors::White );

		renderer2d->DrawString( L"FreeType", { 100.0f,170.0f }, Colors::LightGray );
		renderer2d->DrawString( L"DirectXTex and DXErr", { 100.0f,250.0f }, Colors::LightGray );
		renderer2d->DrawString( L"yaml-cpp", { 100.0f,330.0f }, Colors::LightGray );
		renderer2d->DrawString( L"json (nlohmann)", { 100.0f,410.0f }, Colors::LightGray );

		renderer2d->SelectFont( FontManager::GetFont( "$console" ) );
		renderer2d->DrawString( L"Copyright © 2006-2020 The FreeType Project (www.freetype.org).", { 100.0f,200.0f }, Colors::Gray );
		renderer2d->DrawString( L"All rights reserved.", { 100.0f,215.0f }, Colors::Gray );

		renderer2d->DrawString( L"Copyright © 2011-2020 Microsoft Corporation.", { 100.0f,280.0f }, Colors::Gray );
		renderer2d->DrawString( L"All rights reserved.", { 100.0f,295.0f }, Colors::Gray );

		renderer2d->DrawString( L"Copyright © 2008-2015 Jesse Beder.", { 100.0f,360.0f }, Colors::Gray );
		renderer2d->DrawString( L"All rights reserved.", { 100.0f,375.0f }, Colors::Gray );

		renderer2d->DrawString( L"Copyright © 2013-2021 Niels Lohmann.", { 100.0f,440.0f }, Colors::Gray );
		renderer2d->DrawString( L"All rights reserved.", { 100.0f,455.0f }, Colors::Gray );

		renderer2d->Render();
	}
};
class GameLayer : public ILayer
{
public:
	virtual EventResult OnKeyDown( KeyDownEventArgs&args) override
	{
		if( args.key == Keys::Escape )
		{
			Application::Get()->CloseScene();
			return EventResult::Handled;
		}
		return EventResult::Pass;
	}

	virtual void OnCreate() override
	{
	}
	virtual void OnUpdate() override
	{
		renderer2d->SelectFont( FontManager::GetFont( "credit" ) );
		renderer2d->DrawString( L"There is no game lol", { 300.0f,300.0f }, Colors::White );
		renderer2d->SelectFont( FontManager::GetFont( "$console" ) );
		renderer2d->DrawString( L"(press ESC to go back)", { 300.0f,330.0f }, color(0.3f,0.3f,0.3f) );
		renderer2d->Render();
	}
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

	virtual void OnCreate() override 
	{
		gtimer.reset();
	}
	virtual void OnUpdate() override
	{
		renderer2d->SelectFont( FontManager::GetFont( "$console" ) );
		wm.OnUpdate( gtimer.gets() );
		renderer2d->Render();
	}

	lazer::ui::WindowManager wm;
};

class MenuScene : public Fission::Scene
{
public:
	MenuScene();
};

class CreditsScene : public Fission::Scene
{
public:
	CreditsScene() 
	{ 
		PushLayer( new CreditLayer );

		auto uilayer = new UILayer;
		//  [ adding a window ]         ][ button label ][      what happens when you press button       ][ position on screen
		//		⬇ ⬇ ⬇ ⬇ ⬇                   ⬇ ⬇ ⬇ ⬇                  ⬇ ⬇ ⬇ ⬇ ⬇ ⬇ ⬇ ⬇ ⬇ ⬇ ⬇ ⬇                    ⬇ ⬇ ⬇ ⬇ ⬇
		uilayer->wm.addWindow( new Button( L"Back"     , [] { Fission::Application::Get()->CloseScene(); },   { 110,500 }      ) );
		PushLayer( uilayer );
	}
};

class GameScene : public Fission::Scene
{
public:
	GameScene()
	{
		PushLayer( new GameLayer );

		auto uilayer = new UILayer;
		uilayer->wm.addWindow( new Button( L"oh really?", [] { System::OpenURL( FISSION_Rx2 ); }, { 590,500 } ) );
		PushLayer( uilayer );
	}
};

class MenuLayer : public ILayer
{
public:
	virtual EventResult OnMouseMove( MouseMoveEventArgs & args ) override
	{
		mouse[std::size(mouse)-1] = (vec2f)args.position;
		return EventResult::Pass;
	}

	virtual void OnCreate() override { }
	virtual void OnUpdate() override
	{
		float dt = gtimer.peeks()*2.0f;
		hue += dt, t += dt;

		if( hue >= 1.0f )
			hue = 0.0f;

		auto col = color(hsv_colorf( hue, 1.0f, 1.0f ));

		for( auto i = 0; i < std::size( mouse ); ++i )
		{
			float c = (float)(i+1) / (float)std::size( mouse );
			vec2f diff = ( mouse[i] - center );

			renderer2d->FillArrow( center + diff*0.5f, mouse[i] - diff*0.2f, c*20.0f, col*c );

			if( i < std::size(mouse)-1 )
				mouse[i] = mouse[i + 1];
		}

		renderer2d->SelectFont( FontManager::GetFont( "credit" ) );
		renderer2d->DrawString( L"POV: Budget Game Menu", { 400.0f,200.0f }, Colors::White );

		static float scale = 16.0f;
	//	UI::Debug::Window( "nice" );
		UI::Debug::InputFloat( "scale", &scale );

		static float asd = 41.3f;
		if( scale > 15.0f )
		UI::Debug::InputFloat( "aaa", &asd );

		float k = std::fmodf( t*2.f, std::numbers::pi_v<float>*2.0f );
		float d = std::numbers::pi_v<float> *2.0f - k;
		color cc = color( Colors::White, map( d, 0.0f, std::numbers::pi_v<float>*2.0f, 0.0f, 1.0f ) );
		renderer2d->DrawCircle( { 424.0f,214.0f }, k * scale, color{ 0.0f }, cc * 0.9f, k * 10.0f, StrokeStyle::Outside );
		renderer2d->DrawCircle( { 424.0f,214.0f }, k*scale+ k * 10.0f,cc, cc, d*2.0f, StrokeStyle::Outside );

		renderer2d->DrawCircle( ( mouse[19] - center ) * 0.75f + center, 0.0f, color( col, 0.2f ), color{ 0.0f }, 100.0f, StrokeStyle::Outside );

		renderer2d->Render();
	}
private:
	vec2f mouse[20];
	vec2f center = res * 0.5f;
	float hue = 0.0f;
	float t = 0.0f;
};

MenuScene::MenuScene()
{
	PushLayer( new MenuLayer );
	auto uilayer = new UILayer;
	uilayer->wm.addWindow( new Button( L"Play", [] { Fission::Application::Get()->PushScene( "unused", new GameScene ); }, { 590,340 } ) );
	uilayer->wm.addWindow( new Button( L"Nothing", [] {}, { 590,370 } ) );
	uilayer->wm.addWindow( new Button( L"Credits", [] { Fission::Application::Get()->PushScene( "unused", new CreditsScene ); }, { 590,400 } ) );
	uilayer->wm.addWindow( new Button( L"Exit", [] { Fission::Application::Get()->Exit(); }, { 590,430 } ) );
	PushLayer( uilayer );
}

class SandboxApp : public Application
{
public:
	SandboxApp() : Application( { new MenuScene, u8"sandbox (\U0001f171emo) \U0001f525\U0001f525\U0001f525" } ) {}

	virtual void OnCreate() override
	{
		FontManager::SetFont("credit","assets/Raleway-Regular.ttf",18.0f);

		renderer2d = Renderer2D::Create( GetGraphics() );
	}
};

Application * CreateApplication() {
	return new SandboxApp;
}