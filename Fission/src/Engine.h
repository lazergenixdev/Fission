#pragma once
#include <Fission/Base/Exception.h>
#include <Fission/Base/Utility/SmartPointer.h>

#include <Fission/Core/Engine.hh>
#include <Fission/Core/Application.hh>
#include <Fission/Core/Console.hh>

#include "Platform/GraphicsLoader.h"
#include "Platform/WindowManager.h"

#include "SceneStack.h"

#include "Layer/DebugLayer.h"
#include "Layer/ConsoleLayer.h"

namespace Fission
{
	struct RendererContext
	{
		fsn_ptr<IFRenderer>  renderer;
		bool				 bCreated = false;
	};

	struct FissionEngine : public IFEngine, public IFEventHandler
	{
		fsn_ptr<WindowManager>      m_pWindowManager;
		fsn_ptr<GraphicsLoader>     m_pGraphicsLoader;

		fsn_ptr<IFGraphics>         m_pGraphics;
		fsn_ptr<IFWindow>           m_pWindow;

		std::unordered_map<std::string, RendererContext>
		                            m_Renderers;

		SceneStack                  m_SceneStack;
		DebugLayerImpl				m_DebugLayer;
		ConsoleLayerImpl			m_ConsoleLayer;

		vsync_						m_vsync = vsync_On;

		bool                        m_bRunning = true;
		bool                        m_bMinimized = false;

		std::mutex                  m_PauseMutex;
		std::condition_variable     m_PauseCondition;

		FApplication *              m_Application = nullptr;
		int                         m_ExitCode = 0;



		virtual void GetVersion( int * _Maj, int * _Min, int * _Pat ) override;

		virtual const char * GetVersionString() override;

		virtual void LoadEngine() override;

		virtual void Shutdown( Platform::ExitCode ) override;

		virtual void Run( Platform::ExitCode * e ) override;

		virtual void LoadApplication( FApplication * app ) override;

		virtual void PushScene( FScene * _Ptr_Scene ) override;

		virtual void RegisterRenderer( const char * name, IFRenderer * r ) override;

		virtual IFRenderer * GetRenderer( const char * name ) override;

		virtual IFDebugLayer * GetDebug() override;

		virtual EventResult OnKeyDown( KeyDownEventArgs & )        override;
		virtual EventResult OnKeyUp( KeyUpEventArgs & )            override;
		virtual EventResult OnTextInput( TextInputEventArgs & )    override;
		virtual EventResult OnMouseMove( MouseMoveEventArgs & )    override;
		virtual EventResult OnMouseLeave( MouseLeaveEventArgs & )  override;
		virtual EventResult OnSetCursor( SetCursorEventArgs & )    override;
		virtual EventResult OnHide()                               override;
		virtual EventResult OnShow()                               override;
		virtual EventResult OnClose( CloseEventArgs & )            override;

		virtual void Destroy() override;

		~FissionEngine() = default;
	};

}
