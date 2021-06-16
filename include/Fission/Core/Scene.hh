/**
*
* @file: Scene.h
* @author: lazergenixdev@gmail.com
*
*
* This file is provided under the MIT License:
*
* Copyright (c) 2021 Lazergenix Software
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

// todo: redesign debug layer interface to not be SLOW AF!

#pragma once
#include <Fission/Core/Input/Event.hh>
#include <Fission/Core/Graphics/Renderer2D.hh>

namespace Fission {

	struct IFLayer : public IFEventHandler, public IFObject
	{
		//! @note If you got any resources that need to be sent to the GPU, 
		//!			now is the time to do them
		virtual void OnCreate( class FApplication * app ) = 0;

		//! @brief Function to update what is displayed on a frame
		virtual void OnUpdate() = 0;
		
	}; // struct Fission::IFLayer


/* =================================================================================================== */
/* ----------------------------------------- Default Layers ------------------------------------------ */
/* =================================================================================================== */

	using DebugDrawCallback = std::function<void( IFRenderer2D * pr2d )>;

	class IFDebugLayer : public IFLayer
	{
	public:
		virtual void RegisterDrawCallback( const char * _Key, DebugDrawCallback _Callback ) = 0;

		FISSION_API static void Push( const char * name );

		FISSION_API static void Pop();

		FISSION_API static void Text( const char * what );

		template <size_t Buffer_Size = 128, typename...T>
		static void Text( const char * fmt, T&&...args )
		{
			char _buffer[Buffer_Size];
			sprintf_s( _buffer, fmt, std::forward<T>( args )... );
			Text( _buffer );
		}
	};

	struct IFConsoleLayer : public IFLayer {};

	struct IFUILayer : public IFLayer {};

/* =================================================================================================== */
/* ------------------------------------------ Scene System ------------------------------------------- */
/* =================================================================================================== */

	class FScene : public IFEventHandler, public IFObject
	{
	public:
		virtual void OnCreate( class FApplication * app )
		{
			for( auto && l : m_vUncreatedLayers )
				m_vLayerStack.emplace_back( l );
			for( auto && l : m_vLayerStack )
				l->OnCreate( app );
			m_vUncreatedLayers.clear();
		}

		inline void OnUpdate()
		{
			if( m_vUncreatedLayers.size() )
			{
				for( auto && l : m_vUncreatedLayers )
				{
					l->OnCreate( mApp );
					m_vLayerStack.emplace_back( l );
				}
				m_vUncreatedLayers.clear();
			}
			for( auto && l : m_vLayerStack )
				l->OnUpdate();
		}

		inline void PushLayer( IFLayer * layer )
		{
			m_vUncreatedLayers.emplace_back( layer );
		}

// Very boring code, here only for debugging purposes.
//		"You could make a macro out of this!"
//

		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override
		{
			for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
				if( ( *it )->OnKeyDown( args ) == EventResult::Handled )
					return EventResult::Handled;
			return EventResult::Pass;
		}

		virtual EventResult OnKeyUp( KeyUpEventArgs & args ) override
		{
			for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
				if( ( *it )->OnKeyUp( args ) == EventResult::Handled )
					return EventResult::Handled;
			return EventResult::Pass;
		}

		virtual EventResult OnTextInput( TextInputEventArgs & args ) override
		{
			for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
				if( ( *it )->OnTextInput( args ) == EventResult::Handled )
					return EventResult::Handled;
			return EventResult::Pass;
		}

		virtual EventResult OnMouseMove( MouseMoveEventArgs & args ) override
		{
			for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
				if( ( *it )->OnMouseMove( args ) == EventResult::Handled )
					return EventResult::Handled;
			return EventResult::Pass;
		}

		virtual EventResult OnMouseLeave( MouseLeaveEventArgs & args ) override
		{
			for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
				if( ( *it )->OnMouseLeave( args ) == EventResult::Handled )
					return EventResult::Handled;
			return EventResult::Pass;
		}

		virtual EventResult OnSetCursor( SetCursorEventArgs & args ) override
		{
			for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
				if( ( *it )->OnSetCursor( args ) == EventResult::Handled )
					return EventResult::Handled;
			return EventResult::Pass;
		}

		virtual EventResult OnHide() override
		{
			for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
				( *it )->OnHide();
			return EventResult::Handled;
		}

		virtual EventResult OnShow() override
		{
			for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
				( *it )->OnShow();
			return EventResult::Handled;
		}

		virtual EventResult OnClose( CloseEventArgs & args ) override
		{
			for( auto it = m_vLayerStack.rbegin(); it != m_vLayerStack.rend(); ++it )
				( *it )->OnClose( args );
			return EventResult::Handled;
		}

		virtual ~FScene() 
		{
			FISSION_ASSERT( m_vUncreatedLayers.size() == 0 );
			for( auto && l : m_vLayerStack )
				l->Destroy();
		}

	private:

		friend class SceneStack;

		std::vector<IFLayer *> m_vLayerStack;
		std::vector<IFLayer *> m_vUncreatedLayers;

		class FApplication * mApp = nullptr;
	};

} // nanespace Fission
