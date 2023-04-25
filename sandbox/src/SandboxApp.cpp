#include <Fission/Platform/EntryPoint.h>
#include <Fission/Core/Monitor.hh>
#include <Fission/Base/Time.hpp>
#include <Fission/Simple2DLayer.h>
#include <Fission/Core/Graphics/Font.hh>
#include <Fission/Core/Graphics/Renderer2D.hh>
#include <Fission/Core/Graphics/TextRenderer.hh>
#include <Fission/Core/Graphics.hh>
#include <Fission/Core/Scene.hh>
#include <array>

template <typename T>
struct DefaultDelete : public T { virtual void Destroy() override { delete this; } };

using namespace Fission;

struct MenuSelect {
	static constexpr int n = 5;

	void reset() {
		memcpy( points, rest, std::size(points) * sizeof(float) );
		selected = -1;
		memset( values, 0, std::size(values) * sizeof(float) );
	}

	void init( f32 screen_width ) {
		auto total_width = screen_width * 0.8f;
		auto width = total_width * (1.0f/(float)n);

		auto offset = 0.5f * (screen_width - total_width);
		for( int i = 0; i < std::size(points); ++i )
			points[i] = offset + width * (float)i;

		memcpy(rest, points, std::size(points) * sizeof(float));

		expand = screen_width * 0.05f;

		rest[1] += 40.0f;
		rest[2] += 20.0f;
		rest[3] -= 20.0f;
		rest[4] -= 40.0f;
	}

	std::array<rf32,n> render( Renderer2D* r2d, v2f32 mouse, float dt, bool enable_hitboxes )
	{
		float d = 13.0f * dt;
		selected = -1;

		std::array<rf32, n> rect;
		for( int i = 0; i < n; ++i ) {
			rect[i] = rf32{ points[i], points[i + 1], 50.0f, 150.0f };
		}
		
		bool intersect = false;
		for( int i = 0; i < n; ++i ) {
			if( rect[i][mouse] ) {
				intersect = true;
				for( int k = 0; k <= i; ++k ) {
					points[k] += d * ( (rest[k] - expand) - points[k] );
				}
				for( int k = i+1; k < std::size(points); ++k ) {
					points[k] += d * ( (rest[k] + expand) - points[k] );
				}
				values[i] += d * ( 1.0f - values[i] );
				selected = i;
			}
			else values[i] += 2.0f * d * ( 0.0f - values[i] );
		}
		if( !intersect ) for( int i = 0; i < std::size(points); ++i )
			points[i] += d * ( rest[i] - points[i] );

		else for( int i = 0; i < n; ++i ) {
			rect[i] = rf32{ points[i], points[i + 1], 50.0f, 150.0f };
		}

		if( enable_hitboxes ) {
			for( int i = 0; i < n; ++i ) {
				r2d->DrawRect( rect[i], colors::Aquamarine, 2.0f);
			}
			for( int i = 0; i < std::size(points); ++i ) {
				r2d->DrawRect( rf32::from_center( points[i], 100.0f, 2.0f, 2.0f ), colors::Red, 2.0f );
			}
		}

		return rect;
	}

	float rest   [n+1]; // rest positions
	float points [n+1]; // far left, all points between the rects, and far right

	float expand;
	float values[n] = {};
	int selected = -1;
};

enum MenuState {
	Selection, // selecting which options (Performance, Graphics, ...) to edit
	Editing,   // editing settings for one of the options (Performance, Graphics, ...)
};

struct MenuOptionText {
	const char* str;
	float expand = 30.0f;
	v2f32 offset = {};
	float value = 0.0f;
};

enum Menu {
	Performance,
	Graphics,
	Sound,
	Controls,
	Accessability,
};


struct SettingsScene : public DefaultDelete<Fission::Scene>
{
	virtual void OnCreate( Application* app ) override {
		r2d = app->f_pEngine->GetRenderer<Renderer2D>( "$internal2D" );
		tr = app->f_pEngine->GetRenderer<TextRenderer>( "$Text" );

#define FILE_ "../resources/Fonts/Noto Sans/atlas"

		const void* csv = nullptr;
		auto path = std::filesystem::absolute( FILE_ ".csv" ).make_preferred();
		{
			FILE* f = fopen( path.string().c_str(), "r" );
			fseek( f, 0, SEEK_END );
			long fsize = ftell( f );
			fseek( f, 0, SEEK_SET );  /* same as rewind(f); */
			csv = malloc( fsize );
			fread( (char*)csv, fsize, 1, f );
			fclose( f );
		}
		fnt = SDFFont::Create( { csv, 0, path.replace_extension( "png" ).string().c_str() } );
		app->f_pEngine->RegisterFont( "sdf", fnt );

		auto s = app->f_pMainWindow->GetSize();
		ms.init( (f32)s.w );

		reset_settings_menu();
	}

	void reset_settings_menu()
	{
		ms.reset();
		state = MenuState::Selection;
		MenuOptionText t[] = {
			{"Performance", 24.0f},
			{"Graphics"},
			{"Sound"},
			{"Controls"},
			{"Accessability", 24.0f}
		};
		memcpy(text, t, std::size(t) * sizeof(MenuOptionText));
		show_individual_settings = 0.0f;
	}

	void UpdateMenu()
	{
		if( show_individual_settings > 0.001f ) {
			color c = { colors::White, show_individual_settings };
			
			switch(menu_index)
			{
			case Menu::Performance: {
				auto tl = tr->add_text_sdf( fnt, "Frame Rate", { 50.0f, 50.0f }, 32.0f, c );
				auto rx = rf32{ 300.0f, 400.0f, 50.0f, 50.0f + tl.y }.expand(4.0f);
				float x = 0.2f;
				if( rx[mouse] ) x = 0.4f;
				r2d->FillRect( rx, {colors::White, show_individual_settings*x} );

				tr->add_text_sdf( fnt, "Vertical Sync", { 50.0f, 100.0f }, 32.0f, c );
				tl = tr->add_text_sdf( fnt, "Off", { 300.0f, 100.0f }, 36.0f, c );

				rx = rf32{ 300.0f, 300.0f + tl.x, 100.0f, 100.0f + tl.y }.expand(4.0f);
				if( rx[mouse] )
					r2d->DrawRect( rx, {colors::White, show_individual_settings*0.2f}, 2.0f );

				float offset = 0.0f;
			//	ui_update_dropdown("Frame Rate", offset);
			//	ui_update_onoff("Vertical Sync", offset);
				break;
			}
			case Menu::Graphics: {
				tr->add_text_sdf( fnt, "Graphics settings", { 50.0f, 50.0f }, 32.0f, c );
				break;
			}
			case Menu::Sound: {
				tr->add_text_sdf( fnt, "Sound settings", { 50.0f, 50.0f }, 32.0f, c );
				break;
			}
			case Menu::Controls: {
				tr->add_text_sdf( fnt, "Controls settings", { 50.0f, 50.0f }, 32.0f, c );
				break;
			}
			case Menu::Accessability: {
				tr->add_text_sdf( fnt, "Accessability settings", { 50.0f, 50.0f }, 32.0f, c );
				break;
			}

			default:break;
			}
		}
	}

	virtual void OnUpdate(Fission::timestep dt) override
	{
	//	srand( 2314 );
		for( int i = 0; i < 1000; ++i ) {
			auto c = rgb8( (colors::known)(rand()*7283456827&0xFFFF << 8) );
			r2d->FillRect( rf32::from_center( float( rand() % 1280 ), float( rand() % 720 ), 100.0f, 100.0f ), color{c, 0.1f});
		}

		if( settings ) {

			static constexpr float opacity = 1.0f;
			r2d->FillRectGrad( {0.0f, 1280.0f, 0.0f, 300.0f}, colors::Black, colors::Black, {colors::Black, opacity}, {colors::Black, opacity} );
			r2d->FillRect( {0.0f, 1280.0f, 300.0f, 720.0f}, {colors::Black, opacity} );

			if( state != MenuState::Editing && !button && prev_button && ms.selected >= 0 ) {
				state = MenuState::Editing;
				menu_index = ms.selected;
			}

			if( flag_back ) {
				state = MenuState::Selection;
				flag_back = false;
			}

			std::array<rf32,MenuSelect::n> rects;
			if( state == MenuState::Selection ) {
				rects = ms.render( r2d, mouse, dt, debug );

				for( int i = 0; i < std::size(text); ++i ) {
					text[i].offset += 10.0f * (float)dt * ( v2f32{} - text[i].offset );
					text[i].value += 4.0f * dt * ( 0.0f - text[i].value );
				}

				show_individual_settings += 8.0f * dt * ( 0.0f - show_individual_settings );
			}
			else {
				for( int i = 0; i < MenuSelect::n; ++i ) {
					rects[i] = rf32{ ms.points[i], ms.points[i + 1], 50.0f, 150.0f };
				}

				// Calculate text offsets:
				for( int i = 0; i < std::size(text); ++i ) {
					if( i == ms.selected ) {
						v2f32 want = v2f32(1280.0f - 200.0f, 100.0f) - rects[i].center();
						text[i].offset += 10.0f * (float)dt * ( want - text[i].offset );
						ms.values[i] += 13.0f * dt * ( 1.0f - ms.values[i] );
					}
					else {
						text[i].offset += 10.0f * (float)dt * ( v2f32{} - text[i].offset );
					}
					text[i].value += 10.0f * dt * ( 1.0f - text[i].value );
				}

				show_individual_settings += 4.0f * dt * ( 1.0f - show_individual_settings );
			}


			float size = 34.0f;
			float rest = 0.0f;

			if( state == MenuState::Selection && button && ms.selected >= 0 ) rest = 1.0f;

			hold += 12.0f * dt * ( rest - hold );

			for( int i = 0; i < std::size(rects); ++i ) {
				const auto& r = rects[i];
				const auto& t = text[i];

				static constexpr color base = colors::Gray;
				static constexpr color highlight = colors::GhostWhite;
				color fade = {};

				float scale = size + ms.values[i] * t.expand;
				if( ms.selected == i ) {
					scale -= hold * 8.0f;
					fade = colors::White;
				}

				color col   = base * (1.0f - ms.values[i]) + ms.values[i] * highlight;
				col         = col * ( 1.0f - t.value ) + t.value * fade;

				auto size = tr->text_bounds_sdf(fnt, t.str, scale);
				v2f32 off = 0.5f * (r.sizeVector() - size) + t.offset;
				tr->add_text_sdf( fnt, t.str, r.topLeft() + off, scale, col );

				col = col * ( 1.0f - t.value ) + t.value * color{};
				if( ms.values[i] > 0.01f || t.value < 0.999f ) {
					f32 originx = r.x.center() + t.offset.x;
					f32 originy = r.y.high - off.y * 0.9f + t.offset.y;
					r2d->FillRect( rf32::from_center( originx, originy, ms.values[i] * r.width(), 4.0f), col );
				}
			}

			UpdateMenu();

			prev_button = button;
		}
		else {
			reset_settings_menu();

			tr->add_text_sdf( fnt, "{insert game here}", {150.0f, 300.0f}, 100.0f );
		}
		r2d->Render();
		tr->render();
	}
	virtual Fission::EventResult OnMouseMove(Fission::MouseMoveEventArgs& args) override {
		mouse = (v2f32)args.position;
		return EventResult::Handled;
	}
	virtual Fission::EventResult OnKeyDown(Fission::KeyDownEventArgs& args) override {
		if( !args.repeat )
			switch( args.key ) {
				case Keys::Escape: {
					if( state == MenuState::Editing )
						flag_back = true;
					else
						settings = !settings;
					break;
				}
				case Keys::B: {
					debug = !debug;
					break;
				}
				case Keys::Mouse_Left:
					button = true;
				break;
				default:
				break;
			}
		return EventResult::Handled;
	}
	virtual Fission::EventResult OnKeyUp(Fission::KeyUpEventArgs& args) override {
		switch( args.key ) {
			case Keys::Mouse_Left:
				button = false;
			break;
		default:
			break;
		}
		return EventResult::Handled;
	}
	virtual Fission::SceneKey GetKey() override { return {}; }

	v2f32 mouse = {};
	MenuSelect ms;
	MenuState  state = MenuState::Selection;
	MenuOptionText text[MenuSelect::n];
	float show_individual_settings = 0.0f;

	bool flag_back = false;
	float hold = 0.0f;
	bool button = false;
	bool prev_button = false;
	int menu_index = -1;

	Renderer2D* r2d;
	TextRenderer* tr;
	SDFFont* fnt;

	bool settings = false;
	bool debug    = false;
};

class MyApp : public Fission::Application
{
public:
	MyApp() : Application( "Sandbox", {2,2,8} ) {}

	virtual void OnStartUp( CreateInfo * info ) override
	{
		info->window.title = u8"test settings menu";
		Fission::TextRenderer* tr;
		Fission::CreateTextRenderer( &tr );
		f_pEngine->RegisterRenderer( "$Text", tr );
	}
	virtual Fission::Scene * OnCreateScene( const Fission::SceneKey& key ) override
	{
		// ignore scene key and just create the main scene
		return new SettingsScene;
	}
	virtual void Destroy() override { delete this; }
};

Fission::Application * CreateApplication() {
	return new MyApp;
}