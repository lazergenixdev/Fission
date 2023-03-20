#pragma once
#include <Fission/Base/Exception.hpp>
#include <Fission/Base/util/SmartPointer.hpp>

#include <Fission/Core/Engine.hh>
#include <Fission/Core/Application.hh>
#include <Fission/Core/Scene.hh>
#include <Fission/Core/Console.hh>
#include <Fission/Core/Graphics/Font.hh>

#include "Platform/GraphicsLoader.h"
#include "Platform/WindowManager.h"

#include "Layer/DebugLayer.h"
#include "Layer/ConsoleLayer.h"

namespace Fission
{
	struct RendererContext
	{
		fsn_ptr<Renderer>  renderer;
		bool			   bCreated = false;
	};

	struct FissionEngine : public Engine, public EventHandler
	{
		///////////////////////////////////////////////////////
		fsn_ptr<WindowManager>      m_pWindowManager;
		fsn_ptr<GraphicsLoader>     m_pGraphicsLoader;

		std::vector<SceneKey>		m_SceneKeyHistory;

		fsn_ptr<Graphics>           m_pGraphics;
		fsn_ptr<Window>             m_pWindow;

		std::unordered_map<std::string, RendererContext>
		                            m_Renderers;
		std::unordered_map<std::string, fsn_ptr<Font>>
		                            m_Fonts;

		DebugLayerImpl				m_DebugLayer;
		ConsoleLayerImpl			m_ConsoleLayer;
		Scene *                     m_pCurrentScene;
		Scene *                     m_pNextScene = nullptr;

		vsync_						m_vsync = vsync_On;
		std::optional<color>        m_clearColor = color{};

		bool                        m_bRunning = true;
		bool                        m_bMinimized = false;
		bool                        m_bWantResize = false;

		std::mutex                  m_PauseMutex;
		std::condition_variable     m_PauseCondition;

		Application *               m_Application = nullptr;
		int                         m_ExitCode = 0;

		size2                       m_NewSize;
		///////////////////////////////////////////////////////

	public:

		FissionEngine();

		virtual version GetVersion() override;

		virtual const char * GetVersionString() override;

		virtual void Shutdown( Platform::ExitCode ) override;

		virtual void Run( Platform::ExitCode * e ) override;

		virtual void LoadApplication( Application * app ) override;

		virtual void EnterScene( const SceneKey & key ) override;
		virtual void ExitScene() override;
		virtual void ClearSceneHistory() override;

		virtual void RegisterRenderer( const char * name, Renderer * r ) override;
		virtual Renderer * GetRenderer( const char * name ) override;

		virtual void RegisterFont( const char * name, Font * r ) override;
		virtual Font * GetFont( const char * name ) override;

		virtual DebugLayer * GetDebug() override;

		virtual Graphics* GetGraphics() override;

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
