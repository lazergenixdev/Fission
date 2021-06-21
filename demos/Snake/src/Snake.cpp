#include <Fission/Platform/EntryPoint.h>
#include <Fission/Base/Utility/Timer.h>

template <typename T>
struct DefaultDelete : public T { virtual void Destroy() override { delete this; } };

class SnakeLayer : public DefaultDelete<Fission::IFLayer>
{
};

class GameScene : public DefaultDelete<Fission::FScene>
{
};

class SnakeApp : public DefaultDelete<Fission::FApplication>
{
public:
	virtual void OnStartUp( CreateInfo * info ) override
	{
		char title[100];
		sprintf( title, "Snek Demo [%s]", pEngine->GetVersionString() );
		info->window.title = title;
	}
};

Fission::FApplication * CreateApplication() {
	return new SnakeApp;
}