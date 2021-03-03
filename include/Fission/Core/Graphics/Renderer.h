#pragma once
#include "Fission/config.h"

namespace Fission
{

	struct IRenderer
	{
	public:
	//	virtual void FreeResources() = 0;

		virtual void Render() = 0;
	};

}

