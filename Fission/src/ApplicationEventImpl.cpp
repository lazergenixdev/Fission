#pragma once
#include "Application.h"

namespace Fission {

// Very boring code, here only for debugging purposes
//

	EventResult Application::OnClose( CloseEventArgs & args )
	{
		if( m_State->m_bMinimized )
		{
			m_State->m_bMinimized = false;
			m_State->m_PauseCondition.notify_one();
		}

		m_State->m_ExitCode = args.ExitCode;

#ifndef IMGUI_DISABLE
	{
		std::unique_lock lock( m_State->m_PauseMutex );
		m_State->m_bRunning = false;
		m_State->m_PauseCondition.wait( lock, [this] { return m_State->m_bReadyToExit; } );

		m_State->m_ImGuiLayer.OnClose( args );
#else
		m_State->m_bRunning = false;
#endif

		// notify layers that application will exit
		for( auto && layer : m_State->m_vMainLayers )
			layer->OnClose( args );

#ifndef IMGUI_DISABLE
		m_State->m_bReadyToExit = false;
	}
		m_State->m_PauseCondition.notify_all();
#endif

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
			m_State->m_PauseCondition.notify_one();
		}
		return EventResult::Handled;
	}

	EventResult Application::OnKeyDown( KeyDownEventArgs & args )
	{
		if( m_State->m_DebugLayer.OnKeyDown( args ) == EventResult::Handled )
			return EventResult::Handled;

		if( m_State->m_ConsoleLayer.OnKeyDown( args ) == EventResult::Handled )
			return EventResult::Handled;

#ifndef IMGUI_DISABLE
		if( m_State->m_ImGuiLayer.OnKeyDown( args ) == EventResult::Handled )
			return EventResult::Handled;
#endif

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

#ifndef IMGUI_DISABLE
		if( m_State->m_ImGuiLayer.OnKeyUp( args ) == EventResult::Handled )
			return EventResult::Handled;
#endif

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

#ifndef IMGUI_DISABLE
		if( m_State->m_ImGuiLayer.OnTextInput( args ) == EventResult::Handled )
			return EventResult::Handled;
#endif

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

#ifndef IMGUI_DISABLE
		if( m_State->m_ImGuiLayer.OnMouseMove( args ) == EventResult::Handled )
			return EventResult::Handled;
#endif

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

#ifndef IMGUI_DISABLE
		if( m_State->m_ImGuiLayer.OnMouseLeave( args ) == EventResult::Handled )
			return EventResult::Handled;
#endif

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

#ifndef IMGUI_DISABLE
		if( m_State->m_ImGuiLayer.OnSetCursor( args ) == EventResult::Handled )
			return EventResult::Handled;
#endif

		if( m_State->m_UILayer.OnSetCursor( args ) == EventResult::Handled )
			return EventResult::Handled;

		for( int i = (int)m_State->m_vMainLayers.size() - 1; i >= 0; i-- )
			if( m_State->m_vMainLayers[i]->OnSetCursor( args ) == EventResult::Handled )
				return EventResult::Handled;

		return EventResult::Pass;
	}

}