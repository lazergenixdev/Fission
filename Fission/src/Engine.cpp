#include "Engine.h"
#include "Version.h"

#include "UI/Debug.h"

#define FISSION_ENGINE_ONCE(MSG) \
static bool __bCalled = false; \
if( __bCalled ) FISSION_THROW( "FEngine Error", .append(MSG) ) \
__bCalled = true

#include <Fission/Base/ColoredString.h>
#include <Fission/Platform/System.h>

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

		Console::WriteLine( GetVersionString() / Colors::LightSteelBlue );
		Console::WriteLine( "cmdline: " / Colors::White + GetCommandLineA() / Colors::DimGray );
	}

	Version FissionEngine::GetVersion()
	{
		return { FISSION_VERSION_MAJ, FISSION_VERSION_MIN, FISSION_VERSION_PAT };
	}

	const char * FissionEngine::GetVersionString()
	{
		return FISSION_VERSION_STRV;
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

			if( m_pNextScene )
			{
				auto last = m_pCurrentScene;
				m_pCurrentScene = m_pNextScene;

				last->Destroy();
				m_pNextScene = nullptr;
			}

	/////////////////////////////////////////////////////////////////////////
	// Main Render Loop:

			SwapChain->Bind();
			if( m_clearColor )
			SwapChain->Clear( m_clearColor.value() );

			m_pCurrentScene->OnUpdate( _dt );

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
		app->f_pEngine = this;

		{
			AppCreateInfo appCreateInfo;

			// Fetch start-up information for this app
			app->OnStartUp( &appCreateInfo );

			// Use the app name and version
			{
			char appVersionString[144]; AppCreateInfo * info = &appCreateInfo;
			sprintf_s( appVersionString, "%s " FISSION_VERSION_FORMAT " (" FISSION_VERSION_FORMAT "/%s)",
				app->f_Name.c_str(),
				app->f_Version.Major,
				app->f_Version.Minor,
				app->f_Version.Patch,
				app->f_Version.Major,
				app->f_Version.Minor,
				app->f_Version.Patch,
				app->f_VersionInfo.c_str()
			);
			m_DebugLayer.SetAppVersionString(appVersionString);
			}

			// TODO: this not good.
			m_pCurrentScene = m_Application->OnCreateScene( { 1 } );


			// Create everything needed to run our application:

			m_pGraphicsLoader->CreateGraphics( &appCreateInfo.graphics, &m_pGraphics );
			m_pWindowManager->SetGraphics( m_pGraphics.get() );

			IFWindow::CreateInfo winCreateInfo;
			winCreateInfo.pEventHandler = this;
			winCreateInfo.wProperties = appCreateInfo.window;
			m_pWindowManager->CreateWindow( &winCreateInfo, &m_pWindow );
		}

		app->f_pMainWindow = m_pWindow.get();
		app->f_pGraphics = m_pGraphics.get();

		{
			Fission::IFRenderer2D * renderer;
			Fission::CreateRenderer2D( &renderer );
			RegisterRenderer( "$internal2D", renderer );
		}

		Console::RegisterCommand( "exit", [=]( const string & ) { m_pWindow->Close(); } );

		Console::RegisterCommand("ver", [=](const string&) { Console::WriteLine(FISSION_VERSION_STRV / Colors::White); });

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
		Console::RegisterCommand( "sus", [=]( const string & ) { Console::WriteLine(string("amogus ded")/Colors::MedAquamarine); });

		// Now everything should be initialized, we call OnCreate
		//  for our application and all of its dependencies:

		app->OnCreate();
		m_DebugLayer.OnCreate(app);
		m_ConsoleLayer.OnCreate(app);
		m_pCurrentScene->OnCreate(app);

		base::size wViewportSize = m_pWindow->GetSwapChain()->GetSize();
		for( auto && [name, context] : m_Renderers )
		{
			if( !context.bCreated )
			{
				context.renderer->OnCreate( m_pGraphics.get(), wViewportSize );
				context.bCreated = true;
			}
		}

		CreateDebug( m_pWindowManager.get(), app );
	}


	void FissionEngine::EnterScene( const SceneKey & key )
	{
		if( m_pNextScene == nullptr )
		{
			m_SceneKeyHistory.emplace_back( m_pCurrentScene->GetKey() );

			auto nextScene = m_Application->OnCreateScene( key );

			Console::WriteLine( "Entering New Scene [id=%i] => [id=%i]"_format(m_SceneKeyHistory.back().id, key.id) );

			if( nextScene == nullptr )
			{
				System::ShowSimpleMessageBox( "                       how.", "Invalid Scene", System::Error );
				Shutdown(1);
				return;
			}
			
			nextScene->OnCreate( m_Application );

			m_pNextScene = nextScene;
		}
	}
	void FissionEngine::ExitScene()
	{
		if( m_pNextScene == nullptr )
		{
			auto key = m_SceneKeyHistory.back();
			m_SceneKeyHistory.pop_back();

			auto nextScene = m_Application->OnCreateScene( key );

			Console::WriteLine( "Exiting Scene [id=%i] => [id=%i] "_format(m_pCurrentScene->GetKey().id, key.id) );

			nextScene->OnCreate( m_Application );

			m_pNextScene = nextScene;
		}
	}
	void FissionEngine::ClearSceneHistory()
	{
		m_SceneKeyHistory.clear();
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