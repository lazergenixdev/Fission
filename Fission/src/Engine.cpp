#include "Engine.h"
#include "Version.h"

#include "UI/Debug.h"

#define FISSION_ENGINE_ONCE(MSG) \
static bool bCalled = false; \
if( bCalled ) FISSION_THROW( "FEngine Error", .append(MSG) ) \
bCalled = true

#define _FISSION_FULL_BUILD_STRING FISSION_ENGINE " v" FISSION_FULL_VERSION_STRING

namespace Fission
{
	using namespace string_literals;

	using AppCreateInfo = FApplication::CreateInfo;

	void FissionEngine::GetVersion( int * _Maj, int * _Min, int * _Pat )
	{
		*_Maj = FISSION_VERSION_MAJ;
		*_Min = FISSION_VERSION_MIN;
		*_Pat = FISSION_VERSION_PAT;
	}

	const char * FissionEngine::GetVersionString()
	{
		return _FISSION_FULL_BUILD_STRING;
	}


	void FissionEngine::LoadEngine()
	{
		FISSION_ENGINE_ONCE( "Attempted to call `LoadEngine` more than once." );

		// Initialize Graphics Loader.
		CreateGraphicsLoader( &m_pGraphicsLoader );
		m_pGraphicsLoader->Initialize();

		// Initialize Window Manager.
		CreateWindowManager( &m_pWindowManager );
		m_pWindowManager->Initialize();
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

			m_SceneStack.OnUpdate( m_Application, _dt );

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

		// Memory leak, TODO: fix memory leak.
		AppCreateInfo * appCreateInfo = new AppCreateInfo;

		// Fetch start-up information for this app
		app->OnStartUp( appCreateInfo );

		// Use the app name and version
		{
			char appVersionString[144]; AppCreateInfo * info = appCreateInfo;
			sprintf( appVersionString, "%s %s (%s/vanilla)", info->name_utf8, info->version_utf8, info->version_utf8 );
			m_DebugLayer.SetAppVersionString(appVersionString);
		}

		// Pass our start scene to the scene stack.
		m_SceneStack.PushScene( appCreateInfo->startScene );


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
		m_ConsoleLayer.OnCreate(app);
		m_SceneStack.OnCreate(app);

		base::size wViewportSize = m_pWindow->GetSwapChain()->GetSize();
		for( auto && [name, context] : m_Renderers )
		{
			if( !context.bCreated )
			{
				context.renderer->OnCreate( m_pGraphics.get(), wViewportSize );
				context.bCreated = true;
			}
		}

		Console::RegisterCommand( "exit", [=]( const string & ) { m_pWindow->Close(); return string(); } );

		Console::RegisterCommand( "vsync", 
			[&] ( const string & in ) {
				std::for_each( in.begin(), in.end(), [] ( char8_t & c ) {c = tolower(c);} );

				if( strcmp( in.c_str(), "on" ) == 0 )
				{
					m_vsync = vsync_On;
					return "vsync turned on"_utf8;
				}

				if( strcmp( in.c_str(), "off" ) == 0 )
				{
					m_vsync = vsync_Off;
					return "vsync turned off"_utf8;
				}

				return "incorrect format"_utf8;
			}
		);

		// sus
		Console::RegisterCommand( "sus", [=]( const string & ) { return string("ngl, you kinda be lookin a little SUS!"); });

		CreateDebug( m_pWindowManager.get(), app );
	}


	void FissionEngine::PushScene( FScene * _Ptr_Scene )
	{
		m_SceneStack.OpenScene( _Ptr_Scene );
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
		this->~FissionEngine(); 
		_aligned_free( this ); 
	}



	void CreateEngine( void * instance, IFEngine ** ppEngine )
	{
		void * mem = _aligned_malloc( ( sizeof( FissionEngine ) + 64-sizeof(FissionEngine)%64 ), 64 );
		*ppEngine = new(mem) FissionEngine;
	}
}