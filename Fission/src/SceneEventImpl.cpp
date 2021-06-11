#include <Fission/Core/Scene.hh>

namespace Fission
{
// Very boring code, here only for debugging purposes
//

	EventResult FScene::OnKeyDown( KeyDownEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			if( ( *it )->OnKeyDown( args ) == EventResult::Handled )
				return EventResult::Handled;
		return EventResult::Pass;
	}

	EventResult FScene::OnKeyUp( KeyUpEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			if( ( *it )->OnKeyUp( args ) == EventResult::Handled )
				return EventResult::Handled;
		return EventResult::Pass;
	}

	EventResult FScene::OnTextInput( TextInputEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			if( ( *it )->OnTextInput( args ) == EventResult::Handled )
				return EventResult::Handled;
		return EventResult::Pass;
	}

	EventResult FScene::OnMouseMove( MouseMoveEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			if( ( *it )->OnMouseMove( args ) == EventResult::Handled )
				return EventResult::Handled;
		return EventResult::Pass;
	}

	EventResult FScene::OnMouseLeave( MouseLeaveEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			if( ( *it )->OnMouseLeave( args ) == EventResult::Handled )
				return EventResult::Handled;
		return EventResult::Pass;
	}

	EventResult FScene::OnSetCursor( SetCursorEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			if( ( *it )->OnSetCursor( args ) == EventResult::Handled )
				return EventResult::Handled;
		return EventResult::Pass;
	}

	EventResult FScene::OnHide()
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			( *it )->OnHide();
		return EventResult::Handled;
	}

	EventResult FScene::OnShow()
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			( *it )->OnShow();
		return EventResult::Handled;
	}

	EventResult FScene::OnClose( CloseEventArgs & args )
	{
		for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
			( *it )->OnClose( args );
		return EventResult::Handled;
	}
}