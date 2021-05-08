#include "Fission/Core/Scene.h"

namespace Fission
{
// Very boring code, here only for debugging purposes
//

	EventResult Scene::OnKeyDown( KeyDownEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			if( ( *it )->OnKeyDown( args ) == EventResult::Handled )
				return EventResult::Handled;
		return EventResult::Pass;
	}

	EventResult Scene::OnKeyUp( KeyUpEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			if( ( *it )->OnKeyUp( args ) == EventResult::Handled )
				return EventResult::Handled;
		return EventResult::Pass;
	}

	EventResult Scene::OnMouseMove( MouseMoveEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			if( ( *it )->OnMouseMove( args ) == EventResult::Handled )
				return EventResult::Handled;
		return EventResult::Pass;
	}

	EventResult Scene::OnMouseLeave( MouseLeaveEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			if( ( *it )->OnMouseLeave( args ) == EventResult::Handled )
				return EventResult::Handled;
		return EventResult::Pass;
	}

	EventResult Scene::OnSetCursor( SetCursorEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			if( ( *it )->OnSetCursor( args ) == EventResult::Handled )
				return EventResult::Handled;
		return EventResult::Pass;
	}

}