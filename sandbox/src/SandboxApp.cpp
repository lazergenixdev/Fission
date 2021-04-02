
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
#include "imgui.h"
using namespace Fission;

class SandboxLayer : public ILayer 
{
public:
	SandboxLayer()
	{
		map.Load( "assets/dino_atlas" );

		action = "Walk";
		first = map[action+" (1)"];

		soundengine = SoundEngine::Create();
		sound = soundengine->CreateSound( "assets/sound.mp3" );
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

#ifndef IMGUI_DISABLE
		ImGui::SetCurrentContext( Fission::GetImGuiContext() );
#endif
		source = soundengine->Play( sound.get(), 0 );

		timer.reset();
	}
	virtual void OnUpdate() override
	{
		if( timer.peeks() > 1.0f/(float)animationFR ) ++frame, timer.reset();

		sub_surface::region_uv * region;
		char frameIndex[100];
		sprintf(frameIndex,"%s (%i)",action.c_str(),frame);

		if( auto subs = map[frameIndex] )
			region = &subs->region;
		else
			region = &first->region, frame = 1;

		vec2f res = { 1280,720 };

		static vec2f size = (vec2f)region->abs.size();
#ifndef IMGUI_DISABLE
		ImGui::SetCurrentContext( Fission::GetImGuiContext() );
		static bool show_hitbox = false;
		ImGui::Begin( "Dino" );
		if( ImGui::Button( "Reset" ) )
			size = (vec2f)region->abs.size();
		ImGui::SameLine();
		ImGui::SliderFloat2( "Size", (float *)&size, 20.0f, 500.0f, "%.1f" );
		if( ImGui::SliderFloat( "Animation FPS", &animationFR, 2.0f, 30.0f, "%.1f" ) )
			animationFR = std::clamp( animationFR, 2.0f, 30.0f );
		// could be dynamically loaded from surface map metadata, but too lazy, nobody got time for that.
		static int item = 1;
		static const char * items[] = { "Idle", "Walk", "Run", "Jump", "Dead" };
		ImGui::PushItemWidth( 50.0f );
		ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, { 8.0f, 0.0f } );
		if( ImGui::BeginCombo( "Animation", items[item], ImGuiComboFlags_NoArrowButton ) )
		{
			for( int n = 0; n < std::size( items ); n++ )
			{
				const bool is_selected = ( item == n );
				if( ImGui::Selectable( items[n], is_selected ) )
				{
					item = n;
					std::string str = items[item];
					action = std::move( str );
					frame = 1, timer.reset();
					first = map[action + " (1)"];
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if( is_selected )
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		ImGui::Checkbox( "Show Image Size", &show_hitbox );
		float volume = soundengine->GetMasterVolume();
		if( ImGui::SliderFloat( "Volume", &volume, 0.0f, 1.0f ) )
			soundengine->SetMasterVolume( std::clamp( volume, 0.0f, 1.0f ) );
		{
			float position = source->GetPosition()/1000.0f;
			if( ImGui::SliderFloat( "Position", &position, 0.0f, (float)sound->length() / 1000.0f ) )
				source->SetPosition( position *1000.0f );
		}
		{
			float speed = source->GetPlaybackSpeed();
			if( ImGui::SliderFloat( "", &speed, 0.25f, 4.0f ) )
				source->SetPlaybackSpeed( speed );
		}
		ImGui::SameLine();
		bool playing = source->GetPlaying();
		if( ImGui::Button( playing ? "Pause" : "Play" ) )
			source->SetPlaying( !playing );
		bool vsync = GetApp()->GetGraphics()->GetVSync();
		if( ImGui::Checkbox( "v-sync", &vsync ) )
			GetApp()->GetGraphics()->SetVSync( vsync );
		ImGui::End();
		ImGui::ShowDemoWindow();
#endif
		r2d->DrawImage( tex.get(), rectf::from_center( res/2.0f, size ), region->rel );

#ifndef IMGUI_DISABLE
		if( show_hitbox )
			r2d->DrawRect( rectf::from_center( res/2.0f, size ), Colors::OrangeRed, 2.0f );
#endif

		r2d->Render();
	}
private:
	ref<SoundEngine> soundengine;
	ref<ISound> sound;
	ref<ISoundSource> source;
	surface_map map;
	std::unique_ptr<Renderer2D> r2d;
	std::unique_ptr<Resource::Texture2D> tex;
	std::string action;
	sub_surface * first;
	simple_timer timer;
	int frame = 1;
	float animationFR = 12.3f;
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