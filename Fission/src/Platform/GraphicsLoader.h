#pragma once
#include <Fission/Core/Graphics.hh>

namespace Fission
{
	struct GraphicsLoader : public ManagedObject
	{
		virtual void Initialize() = 0;

		virtual void CreateGraphics( GraphicsState * pState, Graphics ** ppGraphics ) = 0;

		// TODO: make a function to give the reason why an API is not supported.
		virtual bool CheckSupport( Graphics::API api ) = 0;
	};

	extern void CreateGraphicsLoader( GraphicsLoader ** ppGraphicsLoader );
}