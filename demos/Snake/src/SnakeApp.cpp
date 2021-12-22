#include <Fission/Platform/EntryPoint.h>

#include "SnakeApp.h"
#include "GameScene.h"
#include "StartScene.h"

#include <Fission/Core/Sound.hh>

void SnakeApp::OnStartUp( CreateInfo * info )
{
	char title[100];
	sprintf( title, "Snek Demo [%s]", pEngine->GetVersionString() );
	info->window.title = title;
	info->window.size = { 800, 600 };

	strcpy_s(info->name_utf8, "snek");
	strcpy_s(info->version_utf8, "0.2.2");
	
	
	static auto eng = Fission::SoundEngine::Create();
	static auto sound = eng->CreateSound("sphere.mp3");

	static auto source = eng->Play(sound.get(), 0u, true);

	eng->SetMasterVolume(0.25f);
	
	using namespace Fission;
	Console::RegisterCommand("sped",
		[&](const string& in) -> string {
			float value;
			try { value = std::clamp(std::stof(in.string()), 0.05f, 3.0f); }
			catch (...)
			{
				return "Could not determine value.";
			}

			source->SetPlaybackSpeed(value);

			return string("Playback speed set to: ") + std::to_string(value);
		}
	);
	Console::RegisterCommand("goto",
		[&](const string& in) -> string {
			float value;
			try { value = std::stof(in.string()); }
			catch (...)
			{
				return "Could not determine value.";
			}

			source->SetPosition(uint32_t(value*1000.0f));
			
			return string("Playback speed set to: ") + std::to_string(value);
		}
	);

	Fission::Console::WriteLine(
		"r"/Colors::Red
	  + "a"/Colors::Orange
	  + "i"/Colors::Yellow
	  + "n"/Colors::Lime
	  + "b"/Colors::DodgerBlue
	  + "o"/Colors::Indigo
	  + "w"/Colors::Violet
	  + " Console!"/Colors::White
	);
}

Fission::IFScene* SnakeApp::OnCreateScene(const Fission::SceneKey& key)
{
	return new StartScene();
}

Fission::FApplication * CreateApplication() {
	return new SnakeApp;
}