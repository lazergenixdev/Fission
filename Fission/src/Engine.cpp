#include "Engine.h"
#include "Version.h"

#define FISSION_ENGINE_ONCE(MSG) \
static bool __bCalled = false; \
if( __bCalled ) FISSION_THROW( "FEngine Error", .append(MSG) ); \
__bCalled = true

#define FISSION_VERSION_FORMAT "%i.%i.%i"

#include <Fission/Platform/System.h>

namespace Fission
{
	inline Engine* global_engine_pointer = nullptr;

	using AppCreateInfo = Application::CreateInfo;

	FissionEngine::FissionEngine()
	{
		// Initialize Graphics Loader.
		CreateGraphicsLoader(&m_pGraphicsLoader);
		m_pGraphicsLoader->Initialize();

		// Initialize Window Manager.
		CreateWindowManager(&m_pWindowManager);
		m_pWindowManager->Initialize();

		Console::WriteLine( string{GetVersionString()} / colors::LightSteelBlue );
		Console::WriteLine( "cmdline: " / colors::White + string{GetCommandLineA()} / colors::DimGray );
	}

	version FissionEngine::GetVersion()
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

			_dt = frameTimer.gets();
			
	/////////////////////////////////////////////////////////////////////////

		}

		m_Application->OnShutdown();
		m_Renderers.clear();
		*e = m_ExitCode;
	}


	void FissionEngine::LoadApplication( Application * app )
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
			auto app_version = app->f_Version.uncompress();
			char appVersionString[144]; AppCreateInfo * info = &appCreateInfo;
			sprintf_s( appVersionString, "%s " FISSION_VERSION_FORMAT " (" FISSION_VERSION_FORMAT "/%s)",
				app->f_Name.c_str(),
				app_version.Major,
				app_version.Minor,
				app_version.Patch,
				app_version.Major,
				app_version.Minor,
				app_version.Patch,
				app->f_VersionInfo.c_str()
			);
			m_DebugLayer.SetAppVersionString(appVersionString);
			}

			// TODO: this not good.
			m_pCurrentScene = m_Application->OnCreateScene( { 1 } );


			// Create everything needed to run our application:

			m_pGraphicsLoader->CreateGraphics( &appCreateInfo.graphics, &m_pGraphics );
			m_pWindowManager->SetGraphics( m_pGraphics.get() );

			Window::CreateInfo winCreateInfo;
			winCreateInfo.pEventHandler = this;
			winCreateInfo.properties = appCreateInfo.window;
			m_pWindowManager->CreateWindow( &winCreateInfo, &m_pWindow );
		}

		app->f_pMainWindow = m_pWindow.get();
		app->f_pGraphics = m_pGraphics.get();

		{
			Fission::Renderer2D * renderer;
			Fission::CreateRenderer2D( &renderer );
			RegisterRenderer( "$internal2D", renderer );
		}

		Console::RegisterCommand( "exit", [=]( const string & ) { m_pWindow->Close(); } );

		Console::RegisterCommand("ver", [=](const string&) { Console::WriteLine(FISSION_VERSION_STRV / colors::White); });

		Console::RegisterCommand( "vsync", 
			[&] ( string in ) {

				std::for_each( in.data(), in.data()+in.size(), [](char8_t& c) {c = tolower(c); });

				if( in == "on" )
				{
					m_vsync = vsync_On;
					Console::WriteLine( string{"vsync turned on"} );
				}

				if( in == "off" )
				{
					m_vsync = vsync_Off;
					Console::WriteLine( string{"vsync turned off"} );
				}
			}
		);

		// sus
		Console::RegisterCommand( "sus", [=]( const string & ) { Console::WriteLine(string("amogus ded")/colors::MediumAquamarine); });

		// Now everything should be initialized, we call OnCreate
		//  for our application and all of its dependencies:

		app->OnCreate();
		m_DebugLayer.OnCreate(app);
		m_ConsoleLayer.OnCreate(app);
		m_pCurrentScene->OnCreate(app);

		size2 wViewportSize = m_pWindow->GetSize();
		for( auto && [name, context] : m_Renderers )
		{
			if( !context.bCreated )
			{
				context.renderer->OnCreate( m_pGraphics.get(), wViewportSize );
				context.bCreated = true;
			}
		}
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

	void FissionEngine::RegisterRenderer( const char * name, Renderer * r )
	{
		m_Renderers.emplace( name, RendererContext{ r } );
	}

	Renderer * FissionEngine::GetRenderer( const char * name )
	{
		return m_Renderers[name].renderer.get();
	}

	void FissionEngine::RegisterFont( const char* name, Font* f )
	{
		m_Fonts.emplace( name, f );
	}

	Font* FissionEngine::GetFont( const char* name )
	{
		return m_Fonts[name].get();
	}

	DebugLayer * FissionEngine::GetDebug()
	{
		return &m_DebugLayer;
	}

	Graphics* FissionEngine::GetGraphics()
	{
		return m_pGraphics.get();
	}


	void FissionEngine::Destroy() 
	{ 
		delete this;
	}



	void CreateEngine( void * instance, Engine ** ppEngine )
	{
		*ppEngine = new FissionEngine;
		global_engine_pointer = *ppEngine;
	}

	Engine* GetEngine()
	{
		return global_engine_pointer;
	}


	void* _fission_new(size_t _Size) { return ::operator new(_Size, std::align_val_t(32)); }
	void _fission_delete(void* _Ptr) { return ::operator delete(_Ptr, std::align_val_t(32)); }
}