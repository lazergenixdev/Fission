#pragma once
#include <Fission/Core/Scene.hh>
#include "DefaultDelete.h"

class StartMenuLayer : public DefaultDelete<Fission::IFLayer>
{
	void OnCreate(Fission::FApplication*app)
	{
		r2d = (decltype( r2d ))app->pEngine->GetRenderer( "$internal2D" );
	}

	void OnUpdate(Fission::timestep dt)
	{
		r2d->SelectFont( Fission::FontManager::GetFont( "$debug" ) );
		r2d->DrawString( "to be continued...", { 10.0f, 10.0f }, Fission::Colors::White );
		r2d->Render();
	}

	Fission::IFRenderer2D * r2d;
};

class StartScene : public DefaultDelete<Fission::FScene>
{
public:
	StartScene() { PushLayer( new StartMenuLayer ); }
};