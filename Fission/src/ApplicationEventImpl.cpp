#pragma once
#include "Application.h"

namespace Fission {

// Very boring code, here only for debugging purposes
//

	EventResult Application::OnClose( CloseEventArgs & args )
	{
		if( m_State->bMinimized )
		{
			m_State->bMinimized = false;
			m_State->PauseCondition.notify_one();
		}

		m_State->ExitCode = args.ExitCode;

		m_State->SceneStack.OnClose( args );

		m_State->bRunning = false;

		return EventResult::Handled;
	}

	EventResult Application::OnHide()
	{
		m_State->SceneStack.OnHide();

		// Pause the Main thread
		m_State->bMinimized = true;
		return EventResult::Handled;
	}

	EventResult Application::OnShow()
	{
		m_State->SceneStack.OnShow();

		if( m_State->bMinimized )
		{
			m_State->bMinimized = false;
			// Notify Main thread to continue rendering frames
			m_State->PauseCondition.notify_one();
		}
		return EventResult::Handled;
	}

	EventResult Application::OnKeyDown( KeyDownEventArgs & args )
	{
		if( m_State->DebugLayer.OnKeyDown( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->ConsoleLayer.OnKeyDown( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->UILayer.OnKeyDown( args ) == EventResult::Handled )
			return EventResult::Handled;

		return m_State->SceneStack.OnKeyDown( args );
	}

	EventResult Application::OnKeyUp( KeyUpEventArgs & args )
	{
		if( m_State->DebugLayer.OnKeyUp( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->ConsoleLayer.OnKeyUp( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->UILayer.OnKeyUp( args ) == EventResult::Handled )
			return EventResult::Handled;

		return m_State->SceneStack.OnKeyUp( args );
	}

	EventResult Application::OnTextInput( TextInputEventArgs & args )
	{
		if( m_State->DebugLayer.OnTextInput( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->ConsoleLayer.OnTextInput( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->UILayer.OnTextInput( args ) == EventResult::Handled )
			return EventResult::Handled;

		return m_State->SceneStack.OnTextInput( args );
	}

	EventResult Application::OnMouseMove( MouseMoveEventArgs & args )
	{
		if( m_State->DebugLayer.OnMouseMove( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->ConsoleLayer.OnMouseMove( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->UILayer.OnMouseMove( args ) == EventResult::Handled )
			return EventResult::Handled;

		return m_State->SceneStack.OnMouseMove( args );
	}

	EventResult Application::OnMouseLeave( MouseLeaveEventArgs & args )
	{
		if( m_State->DebugLayer.OnMouseLeave( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->ConsoleLayer.OnMouseLeave( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->UILayer.OnMouseLeave( args ) == EventResult::Handled )
			return EventResult::Handled;

		return m_State->SceneStack.OnMouseLeave( args );
	}

	EventResult Application::OnSetCursor( SetCursorEventArgs & args )
	{
		if( m_State->DebugLayer.OnSetCursor( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->ConsoleLayer.OnSetCursor( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->UILayer.OnSetCursor( args ) == EventResult::Handled )
			return EventResult::Handled;

		return m_State->SceneStack.OnSetCursor( args );
	}

}