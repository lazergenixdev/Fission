#include <Fission/Platform/EntryPoint.h>
#include <Fission/Fission.h>
#include <Fission/Core/SurfaceMap.h>
using namespace Fission;

class SandboxLayer : public ILayer 
{
public:
	SandboxLayer()
	{
		map.Load( "assets/dino_atlas" );

		if( map.get_metadata().is_table() )
			action = map.get_metadata()["Action"].as_string();

		first = map[action+" (1)"];

		if( !first ) { 
			Console::Error( L"That is not an action, setting action to \"Walk\"" ); 
			if( !( first = map["Walk (1)"] ) ) exit( 1 );
			action = "Walk";
		}

		Console::RegisterCommand( L"Action", 
			[&] ( std::wstring wstr ) -> std::wstring {
				auto str = wstring_to_utf8( wstr );
				if( map[str + " (1)"] ) {
					action = std::move( str );
					first = map[action+" (1)"];
					Console::WriteLine( Colors::DodgerBlue, L"Loaded the Action: %s", wstr.c_str() );
					return {};
				}
				return wstr + L" is not an action.";
			}
		);
	}

	virtual void OnCreate() override
	{
		auto gfx = GetApp()->GetGraphics();
		r2d = Renderer2D::Create( gfx );

		auto surface = map.release();

		Resource::Texture2D::CreateInfo info;
		info.pSurface = surface.get();
		tex = gfx->CreateTexture2D( info );

		std::wstring wstr = utf8_to_wstring( action );
		Console::WriteLine( Colors::DodgerBlue, L"Loaded the Action: %s", wstr.c_str() );

		timer.reset();
	}
	virtual void OnUpdate() override
	{
		if( timer.peeks() > 0.08f ) ++frame, timer.reset();

		sub_surface::region_uv * region;
		char frameIndex[100];
		sprintf(frameIndex,"%s (%i)",action.c_str(),frame);

		if( auto subs = map[frameIndex] )
			region = &subs->region;
		else
		{
			region = &first->region;
			frame = 1;
		}

		auto size = (vec2f)region->abs.size();
		vec2f res = { 1280,720 };

		r2d->DrawImage( tex.get(), rectf::from_center( res/2.0f, size ), region->rel );

		r2d->Render();
	}
private:
	surface_map map;
	std::unique_ptr<Renderer2D> r2d;
	std::unique_ptr<Resource::Texture2D> tex;
	std::string action;
	sub_surface * first;
	simple_timer timer;
	int frame = 0;
};

class SandboxApp : public Application
{
public:
	SandboxApp() : Application( { L"sandbox" } ) {}

	virtual void OnCreate() override {
		PushLayer( "sandbox", new SandboxLayer );
	}
};

Application * CreateApplication() {
	return new SandboxApp;
}