#include <Fission/Core/Engine.hh>
#include <Fission/Core/Application.hh>
#include <Fission/Base/Exception.h>
#include <Fission/Base/Utility/SmartPointer.h>

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

		std::unordered_map<std::string,RenderContext> m_Renderers;

		bool                        m_bRunning = true;

		SceneStack                  m_SceneStack;
		FApplication *              m_Application = nullptr;

		DebugLayerImpl				m_DebugLayer;
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
			m_pWindow->Close();
		}

		virtual void Run(Platform::ExitCode* e) override
		{
			m_pWindow->GetSwapChain()->Bind();
			while( m_bRunning )
			{
				m_pWindow->GetSwapChain()->Clear( color{} );
				m_SceneStack.OnUpdate(m_Application);
				m_DebugLayer.OnUpdate();
				m_pWindow->GetSwapChain()->Present( vsync_On );
			}
			m_Application->OnShutdown();
			m_Renderers.clear();
			*e = 0x45;
		}

		virtual void LoadApplication( FApplication * app ) override
		{
			FISSION_ENGINE_ONCE( "Attempted to call `LoadApplication` more than once." );

			// Link the application to our engine instance
			m_Application = app;
			app->pEngine = this;

			// Memory leak, TODO: fix memory leak.
			AppCreateInfo * appCreateInfo = new AppCreateInfo;

			// Fetch start-up information for this app
			app->OnStartUp( appCreateInfo );

			// Pass our start scene to the scene stack.
			m_SceneStack.OpenScene( appCreateInfo->startScene );


			// Create everything needed to run our application:

			m_pGraphicsLoader->CreateGraphics( &appCreateInfo->graphics, &m_pGraphics );
			m_pWindowManager->SetGraphics( m_pGraphics.get() );

			IFWindow::CreateInfo winCreateInfo;
			winCreateInfo.pEventHandler = this;
			winCreateInfo.wProperties = appCreateInfo->window;
			m_pWindowManager->CreateWindow( &winCreateInfo, &m_pWindow );

			app->pMainWindow = m_pWindow.get();
			app->pGraphics = m_pGraphics.get();

			{
				Fission::IFRenderer2D * renderer;
				Fission::CreateRenderer2D( &renderer );
				RegisterRenderer( "$internal2D", renderer );
			}

			// Now everything should be initialized, we call OnCreate
			//  for our application and all of its dependencies:

			app->OnCreate();
			m_DebugLayer.OnCreate(app);
			m_SceneStack.OnCreate(app);

			for( auto && [name, context] : m_Renderers )
			{
				if( !context.bCreated )
				{
					context.renderer->OnCreate( m_pGraphics.get() );
					context.bCreated = true;
				}
			}
		}

		virtual void PushScene( FScene * _Ptr_Scene ) override
		{
			m_SceneStack.OpenScene( _Ptr_Scene );
		}

		virtual void RegisterRenderer( const char * name, IFRenderer * r ) override
		{
			m_Renderers.emplace( name, RenderContext{ r } );
		}

		virtual IFRenderer * GetRenderer( const char * name ) override
		{
			return m_Renderers[name].renderer.get();
		}

		virtual EventResult OnClose( CloseEventArgs & args ) override
		{
			m_bRunning = false;
			return EventResult::Handled;
		}

		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override
		{
			if( m_DebugLayer.OnKeyDown( args ) == EventResult::Handled )
				return EventResult::Handled;

			return m_SceneStack.OnKeyDown( args );
		}

		virtual void Destroy() override { delete this; }
	};

	void CreateEngine( void * instance, IFEngine ** ppEngine )
	{
		*ppEngine = new FissionEngine;
	}
}