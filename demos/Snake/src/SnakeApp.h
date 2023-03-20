#pragma once
#include <Fission/Core/Application.hh>
#include <Fission/Base/Time.hpp>
#include "DefaultDelete.h"

class SnakeApp : public DefaultDelete<Fission::Application>
{
public:
	SnakeApp() : DefaultDelete( "Snek", Fission::version{ 1,0,0 } )
	{}

	void OnStartUp( CreateInfo * info ) override;

	Fission::Scene* OnCreateScene( const Fission::SceneKey & key ) override;

	// comment for sake of having comment
private:

	Fission::size2 m_WindowSize;
};
