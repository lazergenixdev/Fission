#include <Fission/Platform/EntryPoint.h>
#include <Fission/Core/Monitor.hh>
#include <Fission/Base/Utility/Timer.h>
#include <Fission/Simple2DLayer.h>

#if defined(DIST)
#define FISSION_ENABLE_DEBUG_UI 0
#else
#define FISSION_ENABLE_DEBUG_UI 1
#endif
#include <Fission/Core/UI/Debug.hh>

static Fission::FApplication * g_App = nullptr;

template <typename T>
struct DefaultDelete : public T { virtual void Destroy() override { delete this; } };

class BallLayer : public DefaultDelete<Fission::Simple2DLayer>
{
public:
	virtual void OnCreate( Fission::FApplication * app ) override
	{
		Simple2DLayer::OnCreate(app);
		wnd = app->pMainWindow;
		font = Fission::FontManager::GetFont("$console");
	}

	virtual void OnUpdate( Fission::timestep dt ) override
	{
		using namespace Fission::base;

		m_pRenderer2D->SelectFont( font );

		float start = 50.0f + pos;
		static char textBuffer[100];
		bool bFoundHover = false;

		static bool value = true;
		Fission::UI::Debug::CheckBox( "purp box", &value );

		static float intger = 9, stok = 2.0f;
		Fission::UI::Debug::SliderFloat( "rad", &intger, 0.0f, 50.0f );
		Fission::UI::Debug::SliderFloat( "stok", &stok, 0.0f, 30.0f );
		if( value && selected )
		{
			m_pRenderer2D->FillRoundRect(
				{ 280.0f, 700.0f, 80.0f, 300.0f },
				intger,
				Fission::color( Fission::Colors::Violet, 0.3f )
			);
			m_pRenderer2D->DrawRoundRect(
				{ 280.0f, 700.0f, 80.0f, 300.0f },
				intger,
				Fission::color( Fission::Colors::Violet, 0.4f ),
				stok,
				Fission::StrokeStyle::Inside
			);
		//	m_pRenderer2D->DrawRect( Fission::base::rectf{ 300.0f, 700.0f, 400.0f, 600.0f }.expanded(0.5f), Fission::Colors::White, 0.5f );
		}

		auto monitor = wnd->GetMonitor();
		if( pos + 20.0f > 0.0f )
		{
			sprintf( textBuffer, "Monitor: %s", monitor->GetName() );
			m_pRenderer2D->DrawString( textBuffer, vector2f{ 12.0f, pos + 2.0f }, Fission::Colors::White );
		}
		for( auto && mode : monitor->GetSupportedDisplayModes() )
		{
			if( start + 30.0f > 0.0f )
			{
				vector2f topleft = { 50.0f, start };
				auto rect = rectf::from_topleft( topleft, 200.0f, 30.0f );

				if( !bFoundHover && rect[mousepos] )
				{
					m_pRenderer2D->FillRect( rect, Fission::Colors::make_gray(0.4f) );
					hover = &mode;
					bFoundHover = true;
				}
				else
				m_pRenderer2D->FillRect( rect, Fission::Colors::make_gray( &mode == selected ? 0.4f : 0.2f ) );

				sprintf( textBuffer, "%i x %i @ %ihz", mode.resolution.w, mode.resolution.h, mode.refresh_rate );
				m_pRenderer2D->DrawString( textBuffer, topleft + vector2f{12.0f, 6.0f}, Fission::Colors::White );

			}
			start += 35.0f;
			if( start > 720.0f ) break;
		}

		if( !bFoundHover ) hover = nullptr;

		if( selected )
		{
			m_pRenderer2D->DrawString( "Selected Display Mode:", { 300.0f, 100.0f }, Fission::Colors::White );

			sprintf( textBuffer, "%i x %i @ %ihz", selected->resolution.w, selected->resolution.h, selected->refresh_rate );
			m_pRenderer2D->DrawString( textBuffer, { 300.0f, 130.0f }, Fission::Colors::Snow );

			m_pRenderer2D->DrawString( "Press [ENTER] to change to this resolution.", { 300.0f, 200.0f }, Fission::Colors::Snow );
		}

		m_pRenderer2D->DrawCircle( mousepos, radius, Fission::Colors::White, Fission::color{ Fission::Colors::White,0.0f }, 100.0f );


		sprintf( textBuffer, "scroll location = %.0f", pos );
		g_App->pEngine->GetDebug()->Text( textBuffer );

		Fission::UI::Debug::SliderFloat( "radius", &radius, 50.0f, 150.0f );
		Fission::UI::Debug::SliderFloat( "scroll_pos", &pos, -2000.0f, 100.0f, "%.0f");

		Fission::UI::Debug::Text( textBuffer );

		if( Fission::UI::Debug::Button( "do thing" ) )
		{
			if( selected ) g_App->pMainWindow->SetSize( selected->resolution );
		}

		m_pRenderer2D->Render();
	}

	virtual Fission::EventResult OnKeyUp( Fission::KeyUpEventArgs & args ) override
	{
		switch( args.key )
		{
		case Fission::Keys::F11:
		{
			if( g_App->pMainWindow->GetStyle() == Fission::IFWindow::Style::Fullscreen )
			{
				g_App->pMainWindow->SetStyle( Fission::IFWindow::Style::Border );
			}
			else
			{
				g_App->pMainWindow->SetStyle( Fission::IFWindow::Style::Fullscreen );
			}
		}
		default:return  Fission::EventResult::Pass;
		}
	}
	virtual Fission::EventResult OnKeyDown( Fission::KeyDownEventArgs & args ) override
	{
		switch( args.key )
		{
		case Fission::Keys::Mouse_WheelUp:
			pos += 10.0f;
		break;

		case Fission::Keys::Mouse_WheelDown:
			pos -= 10.0f;
		break;

		case Fission::Keys::Mouse_Left:
			selected = hover;
		break;

		case Fission::Keys::Enter:
			if( selected ) g_App->pMainWindow->SetSize(selected->resolution);
		break;
		
		default:break;
		}
		return Fission::EventResult::Handled;
	}
	virtual Fission::EventResult OnMouseMove( Fission::MouseMoveEventArgs & args ) override
	{
		mousepos = (Fission::base::vector2f)args.position;
		return Fission::EventResult::Handled;
	}
private:
	Fission::IFWindow * wnd;
	Fission::Font * font;

	float radius = 50.0f;
	float pos = 0.0f;
	Fission::base::vector2f mousepos;

	Fission::DisplayMode * hover = nullptr;
	Fission::DisplayMode * selected = nullptr;
};

class MainScene : public DefaultDelete<Fission::FMultiLayerScene>
{
public:
	MainScene() { PushLayer( new BallLayer ); }

	virtual Fission::SceneKey GetKey() override { return {}; }
};

class BounceBallApp : public DefaultDelete<Fission::FApplication>
{
public:
	virtual void OnStartUp( CreateInfo * info ) override
	{
		info->window.title = u8"🔥 Sandbox 🔥  👌👌👌👌👌";
		strcpy_s( info->name_utf8, "sandbox" );
		strcpy_s( info->version_utf8, "1.0.0" );

	//	pEngine->new_Scene( Fission::SceneKey{ 0x45, {{"user", "Dahrman"}} } );
	}
	virtual Fission::IFScene * OnCreateScene( const Fission::SceneKey& key ) override
	{
		// ignore scene key and just create the main scene
		return new MainScene;
	}
};

Fission::FApplication * CreateApplication() {
	return( g_App = new BounceBallApp );
}