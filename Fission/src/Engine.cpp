#include <Fission/Core/Engine.hh>
#include <Fission/Core/Application.hh>
#include <Fission/Base/Exception.h>

#include "Platform/GraphicsLoader.h"
#include "Platform/WindowManager.h"

#include "SceneStack.h"
#include "Layer/DebugLayer.h"

#include "Version.h"

#define FISSION_ENGINE_ONCE(MSG) \
static bool bCalled = false; \
if( bCalled ) FISSION_THROW( "FEngine Error", .append(MSG) ) \
bCalled = true

#ifdef FISSION_DIST
#define _FISSION_FULL_BUILD_STRING FISSION_ENGINE " v" FISSION_VERSION_STRING
#else
#define _FISSION_FULL_BUILD_STRING FISSION_ENGINE " v" FISSION_VERSION_STRING " " FISSION_BUILD_STRING
#endif

namespace Fission
{
	using AppCreateInfo = FApplication::CreateInfo;

	struct RenderContext
	{
		FPointer<IFRenderer> renderer;
		bool				 bCreated = false;
	};

	struct FissionEngineImpl
	{
		FPointer<WindowManager>     m_pWindowManager;
		FPointer<GraphicsLoader>    m_pGraphicsLoader;

		FPointer<IFGraphics>        m_pGraphics;
		FPointer<IFWindow>          m_pWindow;

		std::vector<FPointer<IFRenderer>> m_Renderers;

		bool                        m_bRunning = true;

		SceneStack                  m_Scenes;
		FApplication *              m_Application = nullptr;

		FPointer<DebugLayerImpl>    m_pDebugLayer;
	};


	struct FissionEngine : public Fission::IFEngine, public FissionEngineImpl, public IFEventHandler
	{
		virtual void GetVersion( int * _Maj, int * _Min, int * _Pat ) override
		{
			*_Maj = FISSION_VERSION_MAJ;
			*_Min = FISSION_VERSION_MIN;
			*_Pat = FISSION_VERSION_PAT;
		}

		virtual const char * GetVersionString() override
		{
			return _FISSION_FULL_BUILD_STRING;
		}

		virtual void LoadEngine() override
		{
			FISSION_ENGINE_ONCE( "Attempted to call `LoadEngine` more than once." );

			// Initialize Graphics Loader.
			CreateGraphicsLoader( &m_pGraphicsLoader );
			m_pGraphicsLoader->Initialize();

			// Initialize Window Manager.
			CreateWindowManager( &m_pWindowManager );
			m_pWindowManager->Initialize();
		}

		virtual void Shutdown( Platform::ExitCode ) override
		{
			FISSION_THROW_NOT_IMPLEMENTED()
		}

		virtual void Run(Platform::ExitCode* e) override
		{
			for( auto && r : m_Renderers )
				r->OnCreate( m_pGraphics.get() );

			m_pWindow->GetSwapChain()->Bind();
			while( m_bRunning )
			{
				m_pWindow->GetSwapChain()->Clear( color{} );
				m_Scenes.OnUpdate();
				m_pDebugLayer->OnUpdate();
				m_pWindow->GetSwapChain()->Present( vsync_On );
			}
			m_Application->OnShutdown();
			m_Renderers.clear();
			*e = 0x45;
		}

		virtual void LoadApplication( FApplication * app ) override
		{
			FISSION_ENGINE_ONCE( "Attempted to call `LoadApplication` more than once." );

			m_Application = app;
			app->pEngine = this;

			// Memory leak, TODO: fix memory leak.
			AppCreateInfo * appCreateInfo = new AppCreateInfo;
			app->OnStartUp( appCreateInfo );
			m_Scenes.OpenScene( appCreateInfo->startScene );

			m_pGraphicsLoader->CreateGraphics( &appCreateInfo->graphics, &m_pGraphics );
			m_pWindowManager->SetGraphics( m_pGraphics.get() );

			m_pDebugLayer = new DebugLayerImpl( m_pGraphics.get() );

			IFWindow::CreateInfo winCreateInfo;
			winCreateInfo.pEventHandler = this;
			winCreateInfo.wProperties = appCreateInfo->window;
			m_pWindowManager->CreateWindow( &winCreateInfo, &m_pWindow );

			app->pMainWindow = m_pWindow.get();
			app->pGraphics = m_pGraphics.get();

			app->OnCreate();
			m_Scenes.OnCreate();
			m_pDebugLayer->OnCreate();
		}

		virtual void PushScene( FScene * _Ptr_Scene ) override
		{
			m_Scenes.OpenScene( _Ptr_Scene );
		}

		virtual void RegisterRenderer( const char * name, IFRenderer * r ) override
		{
			(void)name; // ignored for now
			m_Renderers.push_back( r );
		}

		virtual EventResult OnClose( CloseEventArgs & args ) override
		{
			m_bRunning = false;
			return EventResult::Handled;
		}

		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override
		{
			if( m_pDebugLayer->OnKeyDown( args ) == EventResult::Handled )
				return EventResult::Handled;

			return m_Scenes.OnKeyDown( args );
		}

		virtual void Destroy() override { delete this; }
	};

	void CreateEngine( void * instance, IFEngine ** ppEngine )
	{
		*ppEngine = new FissionEngine;
	}
}