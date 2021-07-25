#pragma once
#include "Debug.h"
#include "DebugWidgets.h"

#include <Fission/Base/Utility/SmartPointer.h>

namespace Fission
{
	struct DebugWindow : public IFLayer, public neutron::DynamicWindow
	{
		std::mutex mutex;
		FPointer<IFRenderer2D> m_pRenderer2D;
		FPointer<IFWindow> window;

		POINT last = {};
		bool mousedown = false;
		base::vector2i mousepos;

		DebugWindow();

		virtual void OnCreate( FApplication * app ) override;
		virtual void OnUpdate( timestep dt ) override;

		DebugWidget * GetWidget( const char * label, int WidgetID, const char * format, const void * pvalue );

		virtual void SetCapture( Window * window ) override;

	//	virtual EventResult OnSetCursor( SetCursorEventArgs & args ) override;
		virtual EventResult OnMouseMove( MouseMoveEventArgs & args ) override;
		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;
		virtual EventResult OnKeyUp( KeyUpEventArgs & args ) override;
		virtual EventResult OnTextInput( TextInputEventArgs & args ) override;


		virtual void Destroy() override;

	private:
		DebugWidget * InsertWidget( const char * label, DebugWidget * widget );
		DebugWidget * InsertTempWidget( DebugWidget * widget );

	private:

		std::vector<DebugWidget *> m_ActiveWidgets;
		std::vector<DebugWidget *> m_TempWidgets;
		std::map<std::string, DebugWidget *> m_Widgets;

		WindowContext context;


		using WidgetAndLabel = decltype(m_Widgets)::value_type;
	};
}
