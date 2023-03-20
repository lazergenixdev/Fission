#pragma once
#include <Fission/Core/Scene.hh>
#include <Fission/Simple2DLayer.h>
#include "DefaultDelete.h"
#include "startui.h"

class StartMenuLayer : public DefaultDelete<Fission::Simple2DLayer>
{
	void OnCreate( Fission::Application * app );

	void OnUpdate( Fission::timestep dt );

	virtual Fission::EventResult OnKeyDown( Fission::KeyDownEventArgs & ) override;
	//virtual Fission::EventResult OnKeyUp( Fission::KeyUpEventArgs & )override;
	virtual Fission::EventResult OnMouseMove( Fission::MouseMoveEventArgs & ) override;
	virtual Fission::EventResult OnSetCursor( Fission::SetCursorEventArgs & ) override;

	Fission::Application * m_App;
	neutron::WindowManager wm;
};

class StartScene : public DefaultDelete<Fission::MultiLayerScene>
{
public:
	StartScene();

	virtual Fission::SceneKey GetKey() override { return {0}; }
};