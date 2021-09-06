#pragma once
#include <Fission/Core/Scene.hh>
#include "DefaultDelete.h"

class StartMenuLayer : public DefaultDelete<Fission::IFLayer>
{
	void OnCreate( Fission::FApplication * app );

	void OnUpdate( Fission::timestep dt );

	//virtual Fission::EventResult OnKeyDown( Fission::KeyDownEventArgs & )override;
	//virtual Fission::EventResult OnKeyUp( Fission::KeyUpEventArgs & )override;
	//virtual Fission::EventResult OnMouseMove( Fission::MouseMoveEventArgs & )override;

	Fission::IFRenderer2D * r2d;
	Fission::FApplication * m_App;
};

class StartScene : public DefaultDelete<Fission::FScene>
{
public:
	StartScene();
};