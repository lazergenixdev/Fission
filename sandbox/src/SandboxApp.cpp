
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

#include <fstream>
#include <random>
#include "lazer/physics.h"

//static vec2f res = { 1920,1080 };
static vec2f res = { 1280,720 };

static std::mt19937 rng;
static std::uniform_real_distribution<float> xdist(0.0f, res.x);
static std::uniform_real_distribution<float> ydist(0.0f, res.y);
static std::uniform_real_distribution<float> sizedist(0.1f, 2.0f);
static std::uniform_real_distribution<float> startdist(-3.141592f, 3.141592f);

static std::uniform_real_distribution<float> dustdist(10.0f, 200.0f);

static std::uniform_real_distribution<float> bigglowdist(10.0f, 50.0f);
static std::uniform_real_distribution<float> glowdist(100.0f, 300.0f);

static std::uniform_real_distribution<float> coinydist(300.0f, 610.0f );
static std::uniform_real_distribution<float> coinxdist(0.0f+10.0f, res.x-10.0f);

static color g_Alpha = Colors::Transparent;

#define TAU 6.28318531f
#define sq(X) ((X)*(X))

static Mesh glowmesh = [] {
	auto mesh = Mesh( 33, 32 );

	static constexpr float m = TAU / 32.0f;

	mesh.push_vertex( { 0.0f,0.0f }, 0 );
	for( auto i = 0; i < 32; ++i )
		mesh.push_vertex( { cosf( m * (float)i ),sinf( m * (float)i ) }, 1 );


	for( auto i = 0; i < 31; ++i )
	{
		mesh.push_index( 0 );
		mesh.push_index( i+1 );
		mesh.push_index( i+2 );
	}
	mesh.push_index( 0 );
	mesh.push_index( 32 );
	mesh.push_index( 1 );

	mesh.push_color( Colors::White );
	mesh.push_color( Colors::Transparent );

	return std::move( mesh );
}();

class glow_particle 
{
public:
	glow_particle()
	{
		size = glowdist( rng );
		pos = { -size / 2.0f,res.y + size / 2.0f };
		vel = vec2f{ xdist( rng ), -ydist( rng ) };
		start = startdist( rng );
	}
	void update( float dt )
	{
		elapsedtime += dt;
		vel -= 0.5f * vel * dt;
		pos += vel * dt;

		if( pos.x > res.x + size || pos.y < -size )
		{
			pos = { -size / 2.0f,res.y + size / 2.0f },
				vel = {xdist(rng), -ydist(rng)};
			elapsedtime = 0.0f;
		}

		if( elapsedtime > 5.0f )
		{
			pos = { -size / 2.0f,res.y + size / 2.0f },
				vel = { xdist( rng ), -ydist( rng ) };
			elapsedtime = 0.0f;
		}
	}
	void draw( Renderer2D * r2d )
	{
		glowmesh.set_color( 0, color( 1.0f, 0.0f, 0.0f, 0.1f - elapsedtime / 50.0f )* g_Alpha );
		r2d->PushTransform( mat3x2f::Transform( size, pos.x, pos.y ) );
		r2d->DrawMesh( &glowmesh );
		r2d->PopTransform();
	}
private:
	vec2f pos;
	vec2f vel;
	float size;
	float start;

	float elapsedtime = 0.0f;
};
class big_particle 
{
public:
	big_particle()
	{
		size = bigglowdist( rng );
		pos = { xdist( rng ), ydist( rng ) };
		vel = vec2f{ xdist( rng ), -ydist( rng ) };
		start = startdist( rng );
	}
	void update( float dt )
	{
		elapsedtime += dt;
		vel -= 0.6f * vel * dt;
		pos += vel * dt;

		if( pos.x > res.x + size || pos.y < -size )
		{
			pos = { -size / 2.0f,res.y + size / 2.0f },
				vel = {xdist(rng), -ydist(rng)};
			elapsedtime = 0.0f;
		}

		if( elapsedtime > 5.0f )
		{
			pos = { -size / 2.0f,res.y + size / 2.0f },
				vel = { xdist( rng ), -ydist( rng ) };
			elapsedtime = 0.0f;
		}
	}
	void draw( Renderer2D * r2d )
	{
		r2d->FillCircle( pos, size, color( 1.0f, 0.0f, 0.0f, 0.05f - elapsedtime / 100.0f )* g_Alpha );
	}
private:
	vec2f pos;
	vec2f vel;
	float size;
	float start;

	float elapsedtime = 0.0f;
};
class dust_particle 
{
public:
	dust_particle()
	{
		pos = { xdist( rng ), ydist( rng ) };
		size = dustdist( rng );
		start = startdist( rng );
	}
	void update( float dt )
	{
		elapsedtime += dt;

		vec2f acc = 50.0f*vec2f((1.0f-sq(pos.x/res.x)),-(1.0f-sq(1.0f-pos.y/res.y)))+vec2f(50.0f,-50.0f);
		
		vel += acc * dt;
		pos += vel * dt;

		if( pos.x > res.x + size ) pos.x = -size/2.0f, vel.x = xdist(rng)/10.0f;
		if( pos.y < -size ) pos.y = res.y + size/2.0f, vel.y = ydist( rng ) / 10.0f;
	}
	void draw( Renderer2D * r2d )
	{
		r2d->FillCircle( pos, size, color( 0.01f, 0.005f, 0.005f, 0.5f+0.5f*sinf(elapsedtime+start) )* g_Alpha );
	}
private:
	vec2f pos;
	vec2f vel;
	float size;
	float start;

	float elapsedtime = 0.0f;

};
class spark_particle 
{
public:
	spark_particle()
	{
		pos = { xdist( rng ), ydist( rng ) };
		size = sizedist( rng );
		start = startdist( rng );
		glow = rand() % 2;
	}
	void update( float dt )
	{
		elapsedtime += dt;

		vec2f acc = 100.0f*vec2f((1.0f-sq(pos.x/res.x)),-(1.0f-sq(1.0f-pos.y/res.y)))+vec2f(100.0f,-100.0f);

		vel += acc * dt;
		pos += vel * dt;

		if( pos.x > res.x ) pos.x = xdist( rng )/2.0f, vel.x = 0.0f, elapsedtime = -start;
		if( pos.y < 0.0f ) pos.y = ydist( rng )/2.0f+360.0f, vel.y = 0.0f, elapsedtime = -start;
	}
	void draw( Renderer2D * r2d )
	{
		auto c = color( Colors::Red, 0.5f + 0.5f * sinf( 5.0f * ( elapsedtime + start ) ) );
		r2d->FillCircle( pos, size, c * g_Alpha );

		if( glow ) {
		glowmesh.set_color( 0, color( c.r, c.g+0.5f, c.b, c.a*0.5f ) * g_Alpha );
		r2d->PushTransform( mat3x2f::Transform( size*4.0f, pos.x, pos.y ) );
		r2d->DrawMesh( &glowmesh );
		r2d->PopTransform();
		}
	}
private:
	vec2f pos;
	vec2f vel;
	float size;
	float start;
	bool glow;

	float elapsedtime = 0.0f;

};

template <typename P>
class ParticleSystem 
{
public:
	ParticleSystem( int n ) { particles.resize( n ); }
	void update( float dt ) { for( auto && p : particles ) p.update( dt ); }
	void draw( Renderer2D * r2d )
	{
		for( auto&& p : particles ) p.draw(r2d);
	}
private:
	std::vector<P> particles;
};

class StatusText
{
public:
	StatusText( const std::string & what ) : m_sWhat( what ) {}

	bool visible() const { return (m_fAlpha > 0.0f); }

	void update(float dt) {
		m_fProgress += dt* sped; // for debugging only

		if( m_fProgress >= 1.0f )
		{
			m_fProgress = 1.0f;
			m_fAlpha -= dt;
		}
	}

	void draw( Renderer2D * r2d, vec2f * base_position, float alpha )
	{
		auto tl = r2d->CreateTextLayout( m_sWhat.c_str() );
		vec2f pos = *base_position;
		vec2f bar_size = { m_fProgress*150.0f, 8.0f};
		auto progress_rect = rectf::from_tl( pos + vec2f( 0.0f, -bar_size.y * 0.5f ), bar_size );
		alpha *= m_fAlpha;

		r2d->DrawString( m_sWhat.c_str(), pos + vec2f( -tl.width - 10.0f, tl.height * -0.5f ), color( m_Color, alpha * m_Color.a * 2.0f ) );
		r2d->FillRect( progress_rect, color( m_Color, alpha * m_Color.a * 0.5f ) );
	  //r2d->DrawRect( progress_rect, color( m_Color, alpha * m_Color.a ), 1.0f );
		r2d->DrawLine( progress_rect.get_bl(), progress_rect.get_br(), color( m_Color, alpha * m_Color.a ), 1.0f );

		{ // End of progress bar thing
			float end = pos.x + 150.0f;
			auto rect = rectf( rangef( end - 1.0f, end ), progress_rect.y );

			color c = ( m_fProgress == 1.0f ? Colors::White : coloru(50,50,50) );
			c.a *= alpha;

			r2d->FillRect( rect, c );

			color glow_color = color( c.r, c.g, c.b, 0.33f*alpha );
			r2d->FillRectGrad( { rect.x.low - 40.0f, rect.x.low, rect.y.low, rect.y.high }, Colors::Transparent, glow_color, Colors::Transparent, glow_color );
			r2d->FillRectGrad( { rect.x.high, rect.x.high + 4.0f, rect.y.low, rect.y.high }, glow_color, Colors::Transparent, glow_color, Colors::Transparent );
		}

		base_position->y += tl.height*(2.0f*(std::clamp(m_fAlpha+0.5f,0.5f, 1.0f)-0.5f));
	}
private:
	float m_fProgress = 0.0f;
	float m_fAlpha = 1.0f;
	std::string m_sWhat;
	color m_Color = color( 0.5f, 0.5f, 0.5f, 0.2f);

	float sped = 0.1f * (float)(rand()%8+1) + 0.004f * (float)( rand() % 50 + 1 );
};

static float alpha0 = 0.262f;
static float alpha1 = 0.502f;
static float alpha2 = 0.867f;

std::string versionstring = [] {
	char buf[100] = {};
	int maj, min, pat;
	GetVersion( &maj, &min, &pat );
	sprintf( buf, "v%i.%i.%i", maj, min, pat );
	return std::string( buf );
}( );

class SandboxLayer : public ILayer 
{
public:
	SandboxLayer()
	{
		Console::SetEnabled( false );
	}

	virtual EventResult OnKeyDown(KeyDownEventArgs&args) override
	{
		if( args.repeat ) return EventResult::Pass;
		switch( args.key )
		{
		case Keys::R:
		{
			t = 0.0f;
			fadeout = 0.0f;
			g_Alpha = Colors::Transparent;
			status.emplace_back( "Loading Textures" );
			status.emplace_back( "Loading Other things" );
			status.emplace_back( "Loading 'non of yo buinis'" );
			status.emplace_back( "Loading Shaders" );
			status.emplace_back( "Loading Sounds" );
			soundengine->SetVolume( 0, 0.5f );
			return EventResult::Handled;
		}
		case Keys::W: { bDoJump = true; return EventResult::Handled; }
		case Keys::A:{ bGoLeft = true; return EventResult::Handled; }
		case Keys::D:{ bGoRight = true; return EventResult::Handled; }
		case Keys::S: { bGoDown = true; return EventResult::Handled; }
		case Keys::Space:
		{
			m = 0.0f;
			return EventResult::Handled;
		}
		case Keys::Q:
		{
			ShellExecuteA( NULL, "open", FISSION_RR, 0, 0, SW_SHOW );
			return EventResult::Handled;
		}
		default:break;
		}
		return EventResult::Pass;
	}
	virtual EventResult OnKeyUp(KeyUpEventArgs&args) override
	{
		switch( args.key )
		{
		case Keys::A: { bGoLeft = false; return EventResult::Handled; }
		case Keys::D: { bGoRight = false; return EventResult::Handled; }
		case Keys::W: { bDoJump = false; return EventResult::Handled; }
		case Keys::S: { bGoDown = false; return EventResult::Handled; }
		case Keys::Space:
		{
			m = 1.0f;
			return EventResult::Handled;
		}
		default:break;
		}
		return EventResult::Pass;
	}
	virtual EventResult OnSetCursor(SetCursorEventArgs&args) override
	{
		//args.cursor = Cursor::Get( Cursor::Default_Hidden );
		return EventResult::Handled;
	}
	virtual void OnCreate() override
	{
		r2d = Renderer2D::Create( GetApp()->GetGraphics() );

		for( auto && pm : Monitor::GetMonitors() )
		{
			Console::WriteLine( 
				L"%s - [%ix%i] @ %ihz", 
				pm->GetName(), 
				pm->GetCurrentDisplayMode()->resolution.x, 
				pm->GetCurrentDisplayMode()->resolution.y, 
				pm->GetCurrentDisplayMode()->refresh_rate 
			);
		}

		r2d->SetBlendMode( BlendMode::Add );

		FontManager::SetFont( "logo", "assets/Raleway-Regular.ttf", 40.0f );
		FontManager::SetFont( "text", "assets/Raleway-Regular.ttf", 12.0f );

		soundengine = SoundEngine::Create( {2} );
		soundengine->SetVolume( 0, 0.5f ); // loading music output
		soundengine->SetVolume( 1, 0.4f ); // game sound fx output
		sound = soundengine->CreateSound( "assets/loading.mp3" );
		coinsound = soundengine->CreateSound( "assets/Pickup_Coin4.wav" );
		source = soundengine->Play( sound.get(), 0, true );

		for( int i = 0; i < 200; i++ ) {
			sparks.update( 0.05f );
			dust.update( 0.05f );
			bigglow.update( 0.05f );
			glow.update( 0.05f );
		}

		for( int i = 0; i < 10; i++ ) {
		status.emplace_back( "Loading Textures" );
		status.emplace_back( "Loading Other things" );
		status.emplace_back( "Loading 'non of yo businis'" );
		status.emplace_back( "Loading Shaders" );
		status.emplace_back( "Loading Sounds" );
		}

		timer.reset();

		//Fission::UI::Debug::Window( "Debug Window" );
		//Fission::UI::Debug::SliderFloat( "what", &m );
		//Fission::UI::Debug::Button( "OK", [] {} );
	}
	virtual void OnUpdate() override
	{
		float dt = m*timer.gets();
		r2d->SetBlendMode( BlendMode::Add );
		//if( dt < 0.2f )
			t += dt;
		if( t > 0.1f )
		{
			float x = std::min( t - 0.1f, 1.0f );
			g_Alpha = color(x, x, x, x);
		}
		
		if( status.empty() )
		{
			fadeout += dt;
			float x = ( 1.0f - fadeout );
			g_Alpha = color( x, x, x, x );
			soundengine->SetVolume( 0, std::max( 0.5f - fadeout, 0.0f ) );
		}
		if( fadeout < 1.0f )
		{
			sparks.update( dt );
			dust.update( dt );
			bigglow.update( dt );
			glow.update( dt );

			r2d->FillRectGrad( {0.0f,res.x, 0.0f,res.y }, color( Colors::Red, alpha0 )* g_Alpha, Colors::Transparent, color( Colors::Red, alpha0 )* g_Alpha, Colors::Transparent );
			r2d->FillTriangleGrad( { 0.0f,200.0f }, { 0.0f,res.y }, { res.x,res.y }, Colors::Transparent, color( Colors::Red, alpha2 )* g_Alpha, Colors::Transparent );
			r2d->FillTriangleGrad( { 0.0f,0.0f }, { 0.0f,res.y }, { 500.0f,res.y }, Colors::Transparent, color( Colors::Yellow, alpha1 )* g_Alpha, Colors::Transparent );

			sparks.draw(r2d.get());
			dust.draw(r2d.get());
			bigglow.draw(r2d.get());
			glow.draw(r2d.get());
		}

		float textalpha = std::clamp( 1.0f*(t - 1.0f), 0.0f, 1.0f );
		if( t >= 4.0f ) { textalpha = std::max( 1.0f - ( t - 4.0f ), 0.0f ); }

		if( t < 5.0f )
		{

			r2d->SelectFont( FontManager::GetFont( "logo" ) );
			auto tl = r2d->CreateTextLayout( "Fission Engine" );
			vec2f textpos = { ( res.x - tl.width ) / 2.0f, ( res.y - tl.height ) / 2.0f };
			r2d->DrawString( "Fission Engine", textpos, color( Colors::White ) * textalpha );

			r2d->SelectFont( FontManager::GetFont( "text" ) );
			r2d->DrawString( "powered by", textpos + vec2f( -10.0f, -15.0f ), color( Colors::White ) * textalpha );

			r2d->SelectFont( FontManager::GetFont( "$console" ) );
			r2d->DrawString( versionstring.c_str(), textpos + vec2f( 0.0f, 55.0f ), color( Colors::Gray, 0.5f ) * textalpha );

		}
		else
		{
			if( fadeout < 1.0f )
			{
				r2d->SelectFont( FontManager::GetFont( "$console" ) );
				vec2f pos = { res.x / 2.0f, 200.0f };
				for( int i = (int)status.size()-1; i >= 0; --i )
				{
					auto & s = status[i];
					s.update( dt );
					if( !s.visible() ) 
						status.erase( status.begin() + i );
				}
				for( auto && s : status )
				{
					s.draw( r2d.get(), &pos, std::clamp( t - 5.0f, 0.0f, 1.0f ) );
				}
			}
			else
			{
				if( !Console::IsEnabled() )
					Console::SetEnabled( true );
				auto spawncoin = [this] () {
					coinpos = { coinxdist( rng ),coinydist( rng ) };
				};

				// update game state
				vel += acc * dt;
				float vx = ( bGoLeft ? ( -1000.0f ) : ( 0.0f ) ) + ( bGoRight ? (1000.0f) : (0.0f) );

				if( bDoJump && pos.y == 610.0f )
					vel.y = -500.0f;

				if( bGoDown )
					vel.y += ( 1000.0f - vel.y ) * 2.0f * dt;

				vel.x += vx * dt;
				vel.x = std::clamp( vel.x, -1000.0f, 1000.0f );
				pos += vel * dt;

				// perform collision checks
				if( pos.y > 620.0f - 10.0f )
				{
					pos.y = 610.0f, vel.y = 0.0f;
					vel.x -= vel.x*dt;
				}

				if( pos.x < 10.0f )
					pos.x = 10.0f, vel.x = 0.0f;
				else if( pos.x > res.x-10.0f )
					pos.x = res.x - 10.0f, vel.x = 0.0f;

				rectf rc = rectf::from_center( pos, { 20.0f, 20.0f } );
				rectf crc = rectf::from_center( coinpos, coinsize );

				aabb bbplayer = aabb( rc.x.low, rc.x.high, rc.y.low, rc.y.high );
				aabb bbcoin = aabb( crc.x.low, crc.x.high, crc.y.low, crc.y.high );
				if( bbplayer ^ bbcoin )
				{
					soundfx.emplace_back( soundengine->Play( coinsound.get(), 1 ) );
					spawncoin();
					++score;
				}
				for( int i = (int)soundfx.size() - 1; i >= 0; --i )
				{
					if( !soundfx[i]->GetPlaying() )
						soundfx.erase( soundfx.begin() + i );
				}

				// draw to screen
				auto sScore = "You own "+std::to_string( score ) + " doge coin, you LOVE doge coin. Collect ALL the doge coin";

				r2d->SelectFont( FontManager::GetFont( "text" ) );
				r2d->DrawString( "WASD to move", { 200.0f, 200.0f }, Colors::White );

				r2d->DrawString( "lol, shit game", { 500.0f, 200.0f }, Colors::White );
				r2d->DrawString( "I made this to test loading screen", { 500.0f, 230.0f }, Colors::White );
				r2d->DrawString( "Press [R] to go back to loading", { 500.0f, 260.0f }, Colors::DarkGray );
				r2d->DrawString( "DON'T PRESS [Q] !!!", { 500.0f, 290.0f }, color(0.25f,0.25f,0.25f) );

				r2d->DrawString( sScore.c_str(), { 0.0f, 0.0f }, Colors::LightYellow );

				r2d->DrawRect( rc, Colors::White, 2.0f, StrokeStyle::Inside ); // player
				float x = ( sinf( t * 5.0f ) * 0.5f + 0.5f );
				float true_width = (coinsize.x-2.0f)*x + 2.0f;
				r2d->FillRect( rectf( rangef::from_center( crc.x.get_average(), true_width ), crc.y ), Colors::Yellow ); // coin

				glowmesh.set_color( 0, color( Colors::Yellow, 0.2f ) );
				r2d->PushTransform( mat3x2f::Transform( vec2f{ std::max( true_width * 3.0f, 18.0f ), coinsize.y * 3.0f }, coinpos.x, coinpos.y ) );
				r2d->DrawMesh( &glowmesh );
				r2d->PopTransform();

				r2d->FillRect( { 0.0f, res.x, 620.0f, 622.0f }, Colors::White );
			}
		}
		r2d->Render();
	}
private:
	scoped<Renderer2D> r2d;
	ParticleSystem<spark_particle> sparks = { 250 };
	ParticleSystem<dust_particle> dust = { 100 };
	ParticleSystem<big_particle> bigglow = { 50 };
	ParticleSystem<glow_particle> glow = { 50 };

	std::vector<StatusText> status;
	simple_timer timer;
	float t = 0.0f;
	float m = 1.0f;

	float fadeout = 0.0f;

	ref<SoundEngine> soundengine;
	ref<ISound> sound;
	ref<ISoundSource> source;

	/* Game Variables */
	vec2f pos = res / 2.0f;
	vec2f vel = {};
	vec2f acc = {0.0f, 300.0f};

	bool bGoLeft = false;
	bool bGoRight = false;
	bool bGoDown = false;
	bool bDoJump = false;

	vec2f coinpos = { 300.0f, 400.0f };
	vec2f coinsize = { 10.0f, 10.0f };
	int score = 0;

	ref<ISound> coinsound;
	std::vector<ref<ISoundSource>> soundfx;
};

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
	DogeWindow( Renderer2D * pr2d ) : DynamicWindow( { 100, 212, 100, 198 } ), m_pr2d(pr2d) {
		id.resize( 64 );
		swprintf_s( &id[0], 64, L"Debug Window"/*, getuid().high, getuid().low */);
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

	virtual void OnCreate() override
	{
		r2d = Renderer2D::Create( GetApp()->GetGraphics() );

		r2d->SelectFont( FontManager::GetFont( "$console" ) );

		auto wnd = new DogeWindow( r2d.get() );
		wnd->addWidget( new Button( L"Exit", r2d.get(), [] { Fission::Application::Get()->Exit(); } ) );
		wnd->addWidget( new Button( L"Nothing", r2d.get(), [] {}, { 10,60 } ) );
		wm.addWindow( wnd );
	}
	virtual void OnUpdate() override
	{
		wm.OnUpdate( 0.0f );
		r2d->Render();
	}

private:
	lazer::ui::WindowManager wm;
	scoped<Renderer2D> r2d;
};

class SandboxApp : public Application
{
public:
	SandboxApp() : Application( { L"sandbox (demo loading screen)" } ) {}

	virtual void OnCreate() override {
		PushLayer( "sandbox", new SandboxLayer );
		PushLayer( "ui", new UILayer );
	}
};

Application * CreateApplication() {
	return new SandboxApp;
}