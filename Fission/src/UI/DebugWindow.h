#pragma once
#include "Debug.h"
#include "DebugWidgets.h"

#include <Fission/Base/Utility/SmartPointer.h>

namespace Fission
{
	struct DebugWindow : public IFLayer, public neutron::DynamicWindow
	{
		std::mutex mutex;
		FPointer<IFRenderer2D> r2d;
		FPointer<IFWindow> window;

		POINT last = {};
		bool mousedown = false;
		base::vector2i mousepos;

		DebugWindow();

		virtual void OnCreate( FApplication * app ) override;
		virtual void OnUpdate( timestep dt ) override;

	//	virtual EventResult OnSetCursor( SetCursorEventArgs & args ) override;
		virtual EventResult OnMouseMove( MouseMoveEventArgs & args ) override;
		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;
		virtual EventResult OnKeyUp( KeyUpEventArgs & args ) override;

		virtual void Destroy() override;
	};
}
