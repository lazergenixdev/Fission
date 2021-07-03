#pragma once
#include <Fission/Core/Scene.hh>
#include <Fission/Core/Application.hh>
#include "DefaultDelete.h"

class GameLayer : public DefaultDelete<Fission::IFLayer>
{
	void OnCreate( Fission::FApplication * app )
	{
		r2d = (decltype( r2d ))app->pEngine->GetRenderer( "$internal2D" );
	}

	void OnUpdate( Fission::timestep dt )
	{
		using namespace Fission::base;

		vector2f offset = { 100.0f, 100.0f };

		r2d->DrawRect( { 0.0f, 600.0f, 0.0f, 600.0f }, Fission::Colors::White, 2.0f, Fission::StrokeStyle::Inside );

		vector2i size = { 10,10 };
		vector2f cellSize = vector2f{ 600.0f,600.0f } / (vector2f)size;

		for( int x = 0; x < size.x; ++x )
		for( int y = 0; y < size.y; ++y )
		{
			r2d->FillRect( rectf::from_topleft( { (float)x * cellSize.x,(float)y * cellSize.y }, cellSize ).expanded( -2.0f ), Fission::color( Fission::Colors::Blue, 0.2f ) );
		}

		r2d->Render();
	}

	Fission::IFRenderer2D * r2d;
};

class GameScene : public DefaultDelete<Fission::FScene>
{
public:
	GameScene() { PushLayer( new GameLayer ); }
};