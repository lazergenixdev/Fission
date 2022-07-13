#include <Fission/Platform/EntryPoint.h>
#include <Fission/Core/Monitor.hh>
#include <Fission/Base/Time.hpp>
#include <Fission/Simple2DLayer.h>

#if defined(DIST)
#define FISSION_ENABLE_DEBUG_UI 0
#else
#define FISSION_ENABLE_DEBUG_UI 1
#endif
#include <Fission/Core/UI/Debug.hh>
#include <Fission/Core/Console.hh>

#define _neutron_key_primary_mouse   Fission::Keys::Mouse_Left
#define _neutron_key_secondary_mouse Fission::Keys::Mouse_Right

#define _neutron_char_type    char32_t
#define _neutron_key_type     Fission::Keys::Key
#define _neutron_cursor_type  Fission::Cursor*

#define _neutron_point_type   Fission::base::vector2i
#define _neutron_rect_type    Fission::base::recti
#define _neutron_vector_type  std::vector
#include <Fission/neutron.hpp>

template <typename T>
struct DefaultDelete : public T { virtual void Destroy() override { delete this; } };

using namespace Fission::base;

static Fission::IFRenderer2D * g_r2d;

class Button : public neutron::Button
{
public:
	vector2f pos, size;
	std::string label;

	Button(const char * label, vector2f pos, vector2f size): pos(pos),size(size),label(label) {}

	virtual bool isInside( neutron::point pos ) override
	{
		rectf rc = rectf::from_topleft( this->pos, size );
		return rc[(vector2f)pos];
	}

	virtual neutron::Result OnSetCursor( neutron::SetCursorEventArgs & args ) override
	{
		args.cursor = Fission::Cursor::Get( Fission::Cursor::Default_Hand );
		return neutron::Handled;
	}

	virtual void OnUpdate(float) override
	{
		auto rect = Fission::base::rectf{ pos.x, pos.x + size.x, pos.y, pos.y + size.y };

		if( parent->GetHover() == this )
		g_r2d->FillRoundRect( rect, 10.0f, Fission::Colors::CadetBlue );
		else
		g_r2d->FillRoundRect( rect, 10.0f, Fission::color(Fission::Colors::CadetBlue,0.5f) );

		g_r2d->DrawRoundRect( rect, 10.0f, Fission::color(Fission::Colors::White,0.5f), 1.0f );

		auto tl = g_r2d->CreateTextLayout( label.c_str() );
		auto start = Fission::base::vector2f{ ( rect.x.distance() - tl.width ) * 0.5f,( rect.y.distance() - tl.height ) * 0.5f } + pos;
		g_r2d->DrawString( label.c_str(), start, Fission::Colors::White );
	}

};

struct Meter
{
	Meter(float damping, float maxv): bar_damping(damping), maxv(maxv)
	{
		x = startx;
		startx += 15.0f;
	}

	Meter& update( float dt )
	{
		if( val < ((float)rand()/(float)RAND_MAX)*100.0f )
			v = maxv;

		val += v * dt;
		v += ( -250.0f - v ) * bar_damping;

		bar += ( val - bar ) * 0.01f;
		bar = std::max( val, bar );

		return *this;
	}

	void draw( Fission::IFRenderer2D *r2d )
	{
		r2d->FillRect( rectf{ x, x+10.0f, 300.0f, 600.0f }, Fission::Colors::make_gray(0.04f) );
		r2d->FillRect( rectf{ x, x+10.0f, 600.0f - val, 600.0f }, Fission::Colors::White );
		r2d->FillRect( rectf{ x, x+10.0f, 594.0f - bar, 597.0f - bar }, Fission::Colors::White );
	}

	float bar_damping = 0.3f;
	float maxv = 8000.0f;

	float val = 10.0f;
	float bar = 10.0f;
	float v = 0.0f;
	float x;

private:
	static float startx;
};

float Meter::startx = 100.0f;

class MenuLayer : public DefaultDelete<Fission::Simple2DLayer>
{
public:
	virtual void OnCreate( Fission::FApplication * app ) override
	{
		Simple2DLayer::OnCreate(app);
		wnd = app->f_pMainWindow;
		font = Fission::FontManager::GetFont("$console");
		g_r2d = m_pRenderer2D;

		{
			auto[x,y] = wnd->GetSize();
			wm.Initialize( x, y );
		}

		wm.addWindow( new Button( "Fake Button", { 100.0f,100.0f }, { 120.0f, 24.0f } ) );
	}

	virtual void OnUpdate( Fission::timestep dt ) override
	{
		m_pRenderer2D->SelectFont( font );
		wm.OnUpdate( 0.0f );
		g_r2d->Render();

		for( auto &meter : meters )
			meter.update(dt).draw(m_pRenderer2D);

		if( show )
		{
			m_pRenderer2D->DrawString( "Showing = True", { 100.0f, 200.0f }, Fission::Colors::White );
		}
		m_pRenderer2D->Render();
	}

	virtual Fission::EventResult OnKeyDown( Fission::KeyDownEventArgs & args ) override
	{
		if( !args.repeat && args.key == Fission::Keys::Escape )
			show =! show; // flip `show`
		return Fission::EventResult::Handled;
	}
	virtual Fission::EventResult OnKeyUp( Fission::KeyUpEventArgs & args ) override
	{
		return Fission::EventResult::Handled;
	}
	virtual Fission::EventResult OnMouseMove( Fission::MouseMoveEventArgs & args ) override
	{
		neutron::MouseMoveEventArgs nargs = { args.position };
		return (Fission::EventResult)wm.OnMouseMove( nargs );
	}
	virtual Fission::EventResult OnSetCursor( Fission::SetCursorEventArgs & args ) override
	{
		neutron::SetCursorEventArgs nargs = { args.cursor };
		auto r = (Fission::EventResult)wm.OnSetCursor( nargs );
		if( nargs.cursor != args.cursor )
		{
			args.cursor = nargs.cursor;
			args.bUseCursor = true;
		}
		return r;
	}
private:
	Fission::IFWindow * wnd;
	Fission::Font * font;

	bool show = false;

	neutron::WindowManager wm;
	Meter meters[4] = { {0.3f,8'000.0f}, {0.4f, 10'000.0f}, {0.25f, 7'500.0f}, {0.5f, 12'000.0f} };
};

class MainScene : public DefaultDelete<Fission::FMultiLayerScene>
{
public:
	MainScene() { PushLayer( new MenuLayer ); }

	virtual Fission::SceneKey GetKey() override { return {}; }
};

class MyApp : public Fission::FApplication
{
public:
	MyApp() : FApplication( "Sandbox", {2,2,8} ) {}

	virtual void OnStartUp( CreateInfo * info ) override
	{
		info->window.title = u8"🔥 Sandbox 🔥  👌👌👌👌👌";
		//strcpy_s( info->name_utf8, "sandbox" );
		//strcpy_s( info->version_utf8, "2.2.0" );
	}
	virtual Fission::IFScene * OnCreateScene( const Fission::SceneKey& key ) override
	{
		//if( auto user = key["user"] )
		//{
		//	Fission::string username = user.value();
		//}

		// ignore scene key and just create the main scene
		return new MainScene;
	}
	virtual void Destroy() override
	{
		delete this;
	}
};

Fission::FApplication * CreateApplication() {
	return new MyApp;
}