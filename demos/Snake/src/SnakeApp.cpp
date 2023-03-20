#include <Fission/Platform/EntryPoint.h>

#include "SnakeApp.h"
#include "GameScene.h"
#include "StartScene.h"

//#include <Fission/Core/Sound.hh>

void SnakeApp::OnStartUp( CreateInfo * info )
{
	char title[100];
	sprintf( title, "Snek Demo [%s]", f_pEngine->GetVersionString() );
	info->window.title = title;
	info->window.size = { 800, 600 };

	using namespace Fission;
	
//	static auto eng = Fission::SoundEngine::Create();
//	static auto sound = eng->CreateSound("sphere.mp3");
//	static auto source = eng->Play(sound.get(), 0u, true);

//	eng->SetMasterVolume(0.25f);
	
	Console::RegisterCommand("sped",
		[&](const string& in) -> string {
			float value;
			try { value = std::clamp(std::stof(in.str()), 0.05f, 3.0f); }
			catch (...)
			{
				return "Could not determine value.";
			}

		//	source->SetPlaybackSpeed(value);

			return cat("Playback speed set to: ", std::to_string(value));
		}
	);
	Console::RegisterCommand("goto",
		[&](const string& in) -> string {
			float value;
			try { value = std::stof(in.str()); }
			catch (...)
			{
				return "Could not determine value.";
			}

		//	source->SetPosition(uint32_t(value*1000.0f));
			
			return cat("Playback speed set to: ", std::to_string(value));
		}
	);

	Fission::Console::WriteLine(
		"r"/colors::Red
	  + "a"/colors::Orange
	  + "i"/colors::Yellow
	  + "n"/colors::Lime
	  + "b"/colors::DodgerBlue
	  + "o"/colors::Indigo
	  + "w"/colors::Violet
	  + " Console!"/colors::White
	);
}

Fission::Scene* SnakeApp::OnCreateScene(const Fission::SceneKey& key)
{
	if( key.id == 0 )
		return new StartScene();
	else
		return new GameScene(f_pEngine);
}

Fission::Application * CreateApplication() {
	return new SnakeApp;
}