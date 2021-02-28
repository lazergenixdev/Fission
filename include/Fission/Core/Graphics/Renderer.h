#pragma once
#include "../../config.h"

namespace lazer
{

	interface IRenderer
	{
	public:
	//	virtual void FreeResources() = 0;

		virtual void Render() = 0;
	};

}

