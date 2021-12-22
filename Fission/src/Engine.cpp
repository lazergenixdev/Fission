#include "Engine.h"
#include "Version.h"

#include "UI/Debug.h"

#define FISSION_ENGINE_ONCE(MSG) \
static bool bCalled = false; \
if( bCalled ) FISSION_THROW( "FEngine Error", .append(MSG) ) \
bCalled = true

#define _FISSION_FULL_BUILD_STRING FISSION_ENGINE " v" FISSION_FULL_VERSION_STRING

#include <Fission/Base/ColoredString.h>

namespace Fission
{
	using namespace string_literals;

	using AppCreateInfo = FApplication::CreateInfo;

	FissionEngine::FissionEngine()
	{
		// Initialize Graphics Loader.
		CreateGraphicsLoader(&m_pGraphicsLoader);
		m_pGraphicsLoader->Initialize();

		// Initialize Window Manager.
		CreateWindowManager(&m_pWindowManager);
		m_pWindowManager->Initialize();

		Fission::Console::WriteLine( GetVersionString() / Colors::LightSteelBlue );
		Fission::Console::WriteLine( "cmdline: " / Colors::White + GetCommandLineA() / Colors::LightGray );
	}

	Version FissionEngine::GetVersion()
	{
		return { FISSION_VERSION_MAJ, FISSION_VERSION_MIN, FISSION_VERSION_PAT };
	}

	const char * FissionEngine::GetVersionString()
	{
		return _FISSION_FULL_BUILD_STRING;
	}


	void FissionEngine::Shutdown( Platform::ExitCode )
	{
		m_pWindow->Close();
	}


	void FissionEngine::Run( Platform::ExitCode * e )
	{
		auto SwapChain = m_pWindow->GetSwapChain();

		timestep _dt;
		auto frameTimer = simple_timer{};

		while( m_bRunning )
		{
			if( m_bMinimized )
			{
				std::unique_lock lock( m_PauseMutex );
				m_PauseCondition.wait( lock );
			}

			if( m_bWantResize )
			{
				for( auto && [name, ctx] : m_Renderers )
					ctx.renderer->OnResize( m_pGraphics.get(), m_NewSize );

				m_pWindow->GetSwapChain()->Resize( m_NewSize );

				m_bWantResize = false;
			}

	/////////////////////////////////////////////////////////////////////////
	// Main Render Loop:

			SwapChain->Bind();
			if( m_clearColor )
			SwapChain->Clear( m_clearColor.value() );

			m_CurrentScene->OnUpdate( _dt );

			m_ConsoleLayer.OnUpdate( _dt );
			m_DebugLayer.OnUpdate( _dt );

			SwapChain->Present( m_vsync );

			RenderDebug(_dt);

			_dt = frameTimer.gets();
			
	/////////////////////////////////////////////////////////////////////////

		}

		m_Application->OnShutdown();
		m_Renderers.clear();
		*e = m_ExitCode;
	}


	void FissionEngine::LoadApplication( FApplication * app )
	{
		FISSION_ENGINE_ONCE( "Attempted to call `LoadApplication` more than once." );

		// Link the application to our engine instance
		m_Application = app;
		app->pEngine = this;

		{
			AppCreateInfo appCreateInfo;

			// Fetch start-up information for this app
			app->OnStartUp( &appCreateInfo );

			// Use the app name and version
			{
			char appVersionString[144]; AppCreateInfo * info = &appCreateInfo;
			sprintf_s( appVersionString, "%s %s (%s/vanilla)", info->name_utf8, info->version_utf8, info->version_utf8 );
			m_DebugLayer.SetAppVersionString(appVersionString);
			}

			// uh??????????????
			m_CurrentScene = m_Application->OnCreateScene( {} );


			// Create everything needed to run our application:

			m_pGraphicsLoader->CreateGraphics( &appCreateInfo.graphics, &m_pGraphics );
			m_pWindowManager->SetGraphics( m_pGraphics.get() );

			IFWindow::CreateInfo winCreateInfo;
			winCreateInfo.pEventHandler = this;
			winCreateInfo.wProperties = appCreateInfo.window;
			m_pWindowManager->CreateWindow( &winCreateInfo, &m_pWindow );
		}

		app->pMainWindow = m_pWindow.get();
		app->pGraphics = m_pGraphics.get();

		{
		Fission::IFRenderer2D * renderer;
		Fission::CreateRenderer2D( &renderer );
		RegisterRenderer( "$internal2D", renderer );
		}

		base::size wViewportSize = m_pWindow->GetSwapChain()->GetSize();
		for( auto && [name, context] : m_Renderers )
		{
			if( !context.bCreated )
			{
				context.renderer->OnCreate( m_pGraphics.get(), wViewportSize );
				context.bCreated = true;
			}
		}

		Console::RegisterCommand( "exit", [=]( const string & ) { m_pWindow->Close(); } );

		Console::RegisterCommand( "vsync", 
			[&] ( const string & in ) {
				std::for_each( in.begin(), in.end(), [] ( char8_t & c ) {c = tolower(c);} );

				if( strcmp( in.c_str(), "on" ) == 0 )
				{
					m_vsync = vsync_On;
					Console::WriteLine( "vsync turned on"_utf8 );
				}

				if( strcmp( in.c_str(), "off" ) == 0 )
				{
					m_vsync = vsync_Off;
					Console::WriteLine( "vsync turned off"_utf8 );
				}
			}
		);

		// sus
		Console::RegisterCommand( "sus", [=]( const string & ) { return string("amogus ded"); });

		// Now everything should be initialized, we call OnCreate
		//  for our application and all of its dependencies:

		app->OnCreate();
		m_DebugLayer.OnCreate(app);
		m_ConsoleLayer.OnCreate(app);
		m_CurrentScene->OnCreate(app);

		CreateDebug( m_pWindowManager.get(), app );
	}


	void FissionEngine::new_Scene( const SceneKey & key )
	{
		//auto oldKey = m_CurrentScene->GetKey();
		//auto scene = m_Application->OnCreateScene( key, &oldKey );
		//m_CurrentScene->Destroy();
		//m_CurrentScene = scene;
	}
	void FissionEngine::back_Scene()
	{
		//m_SceneKeyHistory.pop_back();
		//auto scene = m_Application->OnCreateScene( m_SceneKeyHistory.back() );
		//scene->OnCreate( m_Application );
		//m_CurrentScene->Destroy();
		//m_CurrentScene = scene;
	}
	void FissionEngine::ClearSceneHistory()
	{

	}

	void FissionEngine::RegisterRenderer( const char * name, IFRenderer * r )
	{
		m_Renderers.emplace( name, RendererContext{ r } );
	}

	IFRenderer * FissionEngine::GetRenderer( const char * name )
	{
		return m_Renderers[name].renderer.get();
	}


	IFDebugLayer * FissionEngine::GetDebug()
	{
		return &m_DebugLayer;
	}


	void FissionEngine::Destroy() 
	{ 
		DestroyDebug();
		delete this;
	}



	void CreateEngine( void * instance, IFEngine ** ppEngine )
	{
		*ppEngine = new FissionEngine;
	}


	void* _fission_new(size_t _Size) { return ::operator new(_Size, std::align_val_t(32)); }
	void _fission_delete(void* _Ptr) { return ::operator delete(_Ptr, std::align_val_t(32)); }
}