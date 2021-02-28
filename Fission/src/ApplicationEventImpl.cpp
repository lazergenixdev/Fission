#pragma once
#include "Application.h"

namespace Fission {

// Very boring code, here only for debugging purposes
//

	EventResult Application::OnClose( CloseEventArgs & args )
	{
		// notify layers that application will exit
		for( auto && layer : m_State->m_vMainLayers )
			layer->OnClose( args );

		if( m_State->m_bMinimized )
		{
			m_State->m_bMinimized = false;
			m_State->m_MinimizeCondition.notify_one();
		}

		m_State->m_ExitCode = args.ExitCode;
		m_State->m_bRunning = false;
		return EventResult::Handled;
	}

	EventResult Application::OnHide()
	{
		for( auto && layer : m_State->m_vMainLayers )
			layer->OnHide();

		// Pause the Main thread
		m_State->m_bMinimized = true;
		return EventResult::Handled;
	}

	EventResult Application::OnShow()
	{
		for( auto && layer : m_State->m_vMainLayers )
			layer->OnShow();

		if( m_State->m_bMinimized )
		{
			m_State->m_bMinimized = false;
			// Notify Main thread to continue rendering frames
			m_State->m_MinimizeCondition.notify_one();
		}
		return EventResult::Handled;
	}

	EventResult Application::OnKeyDown( KeyDownEventArgs & args )
	{
		if( m_State->m_DebugLayer.OnKeyDown( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_ConsoleLayer.OnKeyDown( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_UILayer.OnKeyDown( args ) == EventResult::Handled )
			return EventResult::Handled;

		for( int i = (int)m_State->m_vMainLayers.size() - 1; i >= 0; i-- )
			if( m_State->m_vMainLayers[i]->OnKeyDown( args ) == EventResult::Handled )
				return EventResult::Handled;

		return EventResult::Pass;
	}

	EventResult Application::OnKeyUp( KeyUpEventArgs & args )
	{
		if( m_State->m_DebugLayer.OnKeyUp( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_ConsoleLayer.OnKeyUp( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_UILayer.OnKeyUp( args ) == EventResult::Handled )
			return EventResult::Handled;

		for( int i = (int)m_State->m_vMainLayers.size() - 1; i >= 0; i-- )
			if( m_State->m_vMainLayers[i]->OnKeyUp( args ) == EventResult::Handled )
				return EventResult::Handled;

		return EventResult::Pass;
	}

	EventResult Application::OnTextInput( TextInputEventArgs & args )
	{
		if( m_State->m_DebugLayer.OnTextInput( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_ConsoleLayer.OnTextInput( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_UILayer.OnTextInput( args ) == EventResult::Handled )
			return EventResult::Handled;

		for( int i = (int)m_State->m_vMainLayers.size() - 1; i >= 0; i-- )
			if( m_State->m_vMainLayers[i]->OnTextInput( args ) == EventResult::Handled )
				return EventResult::Handled;

		return EventResult::Pass;
	}

	EventResult Application::OnMouseMove( MouseMoveEventArgs & args )
	{
		if( m_State->m_DebugLayer.OnMouseMove( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_ConsoleLayer.OnMouseMove( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_UILayer.OnMouseMove( args ) == EventResult::Handled )
			return EventResult::Handled;

		for( int i = (int)m_State->m_vMainLayers.size() - 1; i >= 0; i-- )
			if( m_State->m_vMainLayers[i]->OnMouseMove( args ) == EventResult::Handled )
				return EventResult::Handled;

		return EventResult::Pass;
	}

	EventResult Application::OnMouseLeave( MouseLeaveEventArgs & args )
	{
		if( m_State->m_DebugLayer.OnMouseLeave( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_ConsoleLayer.OnMouseLeave( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_UILayer.OnMouseLeave( args ) == EventResult::Handled )
			return EventResult::Handled;

		for( int i = (int)m_State->m_vMainLayers.size() - 1; i >= 0; i-- )
			if( m_State->m_vMainLayers[i]->OnMouseLeave( args ) == EventResult::Handled )
				return EventResult::Handled;

		return EventResult::Pass;
	}

	EventResult Application::OnSetCursor( SetCursorEventArgs & args )
	{
		if( m_State->m_DebugLayer.OnSetCursor( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_ConsoleLayer.OnSetCursor( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_UILayer.OnSetCursor( args ) == EventResult::Handled )
			return EventResult::Handled;

		for( int i = (int)m_State->m_vMainLayers.size() - 1; i >= 0; i-- )
			if( m_State->m_vMainLayers[i]->OnSetCursor( args ) == EventResult::Handled )
				return EventResult::Handled;

		return EventResult::Pass;
	}

}