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

#pragma once
#include <Fission/Core/Layer.hh>
#include <Fission/Core/Graphics/Renderer2D.hh>

namespace Fission
{
	using SceneID = int;
	
	static constexpr SceneID NullSceneID     =  0; // reserved ID, used to represent no scene
	static constexpr SceneID CMDLineSceneID  = -1; // reserved ID, used to represent scene defined by cmd line
	static constexpr SceneID ExternalSceneID = -2; // reserved ID, used to represent external scene


	struct SceneKey : public std::map<string, string>
	{
		using map = std::map<string, string>;
		using arg_list = std::initializer_list<value_type>;
		SceneID id;

		SceneKey( SceneID _ID = NullSceneID ) : map(), id( _ID )
		{}

		SceneKey( SceneID _ID, const arg_list & _Args ): map( _Args ), id(_ID)
		{}

	};

	struct IFScene : public IFLayer
	{
		virtual void OnCreate( class FApplication * app ) override {}

		virtual void OnUpdate( timestep dt ) override {}

		virtual SceneKey GetKey() = 0;

	};

	struct FMultiLayerScene : public IFScene
	{
	public:
		// This should be called after all layers have been pushed onto the layer stack.
		virtual void OnCreate( class FApplication * app ) override
		{
			for( auto && l : m_vLayerStack )
				l->OnCreate( app );
		}

		virtual void OnUpdate( timestep dt ) override
		{
			for( auto && l : m_vLayerStack )
				l->OnUpdate( dt );
		}

		inline void PushLayer( IFLayer * layer )
		{
			m_vLayerStack.emplace_back( layer );
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

		virtual ~FMultiLayerScene() 
		{
			for( auto && l : m_vLayerStack )
				l->Destroy();
		}

	private:

		friend class SceneStack;

		std::vector<IFLayer *> m_vLayerStack;

		class FApplication * mApp = nullptr;

	}; // class Fission::FScene

} // nanespace Fission
