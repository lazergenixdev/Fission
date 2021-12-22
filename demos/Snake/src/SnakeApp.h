#pragma once
#include <Fission/Core/Application.hh>
#include <Fission/Base/Utility/Timer.h>
#include "DefaultDelete.h"

class SnakeApp : public DefaultDelete<Fission::FApplication>
{
public:
	void OnStartUp( CreateInfo * info ) override;

	Fission::IFScene* OnCreateScene( const Fission::SceneKey & key ) override;

	// comment for sake of having comment
private:

	Fission::base::size m_WindowSize;
};
