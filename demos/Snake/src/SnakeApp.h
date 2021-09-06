#pragma once
#include <Fission/Core/Application.hh>
#include <Fission/Base/Utility/Timer.h>
#include "DefaultDelete.h"

class SnakeApp : public DefaultDelete<Fission::FApplication>
{
public:
	void OnStartUp( CreateInfo * info );

	Fission::base::size GetWindowSize() const;

	// comment for sake of having comment
private:

	Fission::base::size m_WindowSize;
};
