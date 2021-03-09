
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

#define MOVE_UP_BIT 0b0001
#define MOVE_DOWN_BIT 0b0010
#define MOVE_LEFT_BIT 0b0100
#define MOVE_RIGHT_BIT 0b1000

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

	virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override { 
		if( !args.repeat )
		switch( args.key )
		{
		case Keys::Up: moveFlags |= MOVE_UP_BIT; break;
		case Keys::Down: moveFlags |= MOVE_DOWN_BIT; break;
		case Keys::Left: moveFlags |= MOVE_LEFT_BIT; break;
		case Keys::Right: moveFlags |= MOVE_RIGHT_BIT; break;
		default:
			break;
		}
		return FISSION_EVENT_HANDLED;
	}

	virtual EventResult OnKeyUp( KeyUpEventArgs & args ) override { 
		switch( args.key )
		{
		case Keys::Up: moveFlags &=~ MOVE_UP_BIT; break;
		case Keys::Down: moveFlags &=~ MOVE_DOWN_BIT; break;
		case Keys::Left: moveFlags &=~ MOVE_LEFT_BIT; break;
		case Keys::Right: moveFlags &=~ MOVE_RIGHT_BIT; break;
		default:
			break;
		}
		return FISSION_EVENT_HANDLED;
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
		if( moveFlags & MOVE_UP_BIT ) off.y -= 1.0f;
		if( moveFlags & MOVE_DOWN_BIT ) off.y += 1.0f;
		if( moveFlags & MOVE_LEFT_BIT ) off.x -= 1.0f;
		if( moveFlags & MOVE_RIGHT_BIT ) off.x += 1.0f;

		if( timer.peeks() > 1.0f/(float)animationFR ) ++frame, timer.reset();

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
		static int item = 1;
		static const char * items[] = {
			"Idle",
			"Walk",
			"Run",
			"Jump",
			"Dead",
		};
		if( ImGui::Combo( "Animation", &item, items, (int)std::size( items ) ) )
		{
			std::string str = items[item];
			action = std::move( str );
			frame = 1, timer.reset();
			first = map[action + " (1)"];
		}
		ImGui::Checkbox( "Show Image Size", &show_hitbox );
		static char buffer[1000] = {};
		ImGui::InputTextMultiline( "Text\nthing", buffer, 1000 );
		float volume = soundengine->GetMasterVolume();
		if( ImGui::SliderFloat( "Volume", &volume, 0.0f, 1.0f ) )
			soundengine->SetMasterVolume( std::clamp( volume, 0.0f, 1.0f ) );
		bool playing = source->GetPlaying();
		if( ImGui::Button( playing ? "Pause" : "Play" ) )
			source->SetPlaying( !playing );
		bool vsync = GetApp()->GetGraphics()->GetVSync();
		if( ImGui::Checkbox( "v-sync", &vsync ) )
			GetApp()->GetGraphics()->SetVSync( vsync );
		ImGui::End();
		ImGui::ShowDemoWindow();
#endif
		auto sz = size;
		if( moveFlags & MOVE_LEFT_BIT ) sz.x = -size.x;
		r2d->DrawImage( tex.get(), rectf::from_center( res/2.0f+off, sz ), region->rel );

#ifndef IMGUI_DISABLE
		if( show_hitbox )
			r2d->DrawRect( rectf::from_center( res/2.0f+off, size ), Colors::OrangeRed, 2.0f );
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
	vec2f off;
	unsigned int moveFlags = 0;
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