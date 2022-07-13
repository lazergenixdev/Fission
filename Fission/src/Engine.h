#pragma once
#include <Fission/Base/Exception.hpp>
#include <Fission/Base/util/SmartPointer.hpp>

#include <Fission/Core/Engine.hh>
#include <Fission/Core/Application.hh>
#include <Fission/Core/Scene.hh>
#include <Fission/Core/Console.hh>

#include "Platform/GraphicsLoader.h"
#include "Platform/WindowManager.h"

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
		///////////////////////////////////////////////////////
		fsn_ptr<WindowManager>      m_pWindowManager;
		fsn_ptr<GraphicsLoader>     m_pGraphicsLoader;

		std::vector<SceneKey>		m_SceneKeyHistory;

		fsn_ptr<IFGraphics>         m_pGraphics;
		fsn_ptr<IFWindow>           m_pWindow;

		std::unordered_map<std::string, RendererContext>
		                            m_Renderers;

		DebugLayerImpl				m_DebugLayer;
		ConsoleLayerImpl			m_ConsoleLayer;
		IFScene *                   m_pCurrentScene;
		IFScene *                   m_pNextScene = nullptr;

		vsync_						m_vsync = vsync_On;
		std::optional<color>        m_clearColor = color{};

		bool                        m_bRunning = true;
		bool                        m_bMinimized = false;
		bool                        m_bWantResize = false;

		std::mutex                  m_PauseMutex;
		std::condition_variable     m_PauseCondition;

		FApplication *              m_Application = nullptr;
		int                         m_ExitCode = 0;

		size2                  m_NewSize;
		///////////////////////////////////////////////////////

	public:

		FissionEngine();

		virtual version GetVersion() override;

		virtual const char * GetVersionString() override;

		virtual void Shutdown( Platform::ExitCode ) override;

		virtual void Run( Platform::ExitCode * e ) override;

		virtual void LoadApplication( FApplication * app ) override;

		virtual void EnterScene( const SceneKey & key ) override;
		virtual void ExitScene() override;
		virtual void ClearSceneHistory() override;

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
		virtual EventResult OnResize( ResizeEventArgs & )          override;

		virtual void Destroy() override;

		~FissionEngine() = default;
	};

}
