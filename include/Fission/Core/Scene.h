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
#include "Input/Event.h"
#include "Graphics/Renderer2D.h"

namespace Fission {

	class Application;

	interface ILayer : public IEventHandler
	{
		virtual void OnCreate() = 0;
		virtual void OnUpdate() = 0;

		virtual ~ILayer() noexcept = default;

	protected:
		
		FISSION_API Application * GetApp();
		
	}; // interface Fission::ILayer


	///////////////////////////////////////////////////////////////////////
	// Default Layers /////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////

	using DebugDrawCallback = std::function<void( Renderer2D * pr2d )>;

	class DebugLayer : public ILayer
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

	interface IConsoleLayer : public ILayer
	{
	};

	interface IUILayer : public ILayer
	{
		
	};
	
	class Scene : public IEventHandler
	{
	public:
		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;

		void OnUpdate()
		{
		}

		void PushLayer( ILayer * );

		void Close();
		void Open( Scene * scene );

	private:
		std::vector<ILayer *> m_vLayerStack;
	};

	class SceneStack : public IEventHandler
	{
	public:

		void OnUpdate()
		{
			m_vSceneStack.back()->OnUpdate();
		}

		Scene * front();


	private:
		float m_SceneSwitchCooldownDuration = 1.0f;
		simple_timer m_SceneSwitchTimer;
		bool m_bSceneSwitch = false;

		std::vector<Scene *> m_vSceneStack;
	};

} // nanespace Fission
