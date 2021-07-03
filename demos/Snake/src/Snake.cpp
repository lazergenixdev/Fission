#include <Fission/Platform/EntryPoint.h>
#include <Fission/Base/Utility/Timer.h>

#include "StartScene.h"
#include "GameScene.h"

class SnakeApp : public DefaultDelete<Fission::FApplication>
{
public:
	void OnStartUp( CreateInfo * info )
	{
		char title[100];
		sprintf( title, "Snek Demo [%s]", pEngine->GetVersionString() );
		info->window.title = title;
		info->window.size = { 800, 600 };
		info->startScene = new StartScene;
	}
};

Fission::FApplication * CreateApplication() {
	return new SnakeApp;
}