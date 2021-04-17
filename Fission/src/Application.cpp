#pragma once
#include "Application.h"
#include "Fission/Core/Console.h"
#include "Fission/Core/Configuration.h"

using namespace Fission;

static Application * s_pInstance = nullptr;

static float s_LastDelta = 0.0f;
static simple_timer s_AppTimer;

Application * ILayer::GetApp() {
	return s_pInstance;
}


static void RegisterConsoleCommands()
{
	Console::RegisterCommand(
		L"exit",
		[] ( std::wstring ) -> std::wstring {
			Application::Get()->Exit();
			return L"exiting ..";
		}
	);
	Console::RegisterCommand(
		L"vsync",
		[] ( std::wstring s ) -> std::wstring {
		if( s == L"on" )
		{
			Application::Get()->GetGraphics()->SetVSync( vsync_On );
			return L"vsync turned on";
		}
		else if( s == L"off" )
		{
			Application::Get()->GetGraphics()->SetVSync( vsync_Off );
			return L"vsync turned off";
		}
		else
			return L"incorrect format";
		return L"";
		}
	);
}

Application * Application::Get() {
	return s_pInstance;
}

Application::Application( const CreateInfo & info ) 
	: m_State( new ApplicationState )
{
	if( s_pInstance )
		throw std::logic_error( "more than one instance? interesting, but no" );

	s_pInstance = this;

	Console::WriteLine( Colors::Aqua, L"Application started!" );

//	System::WriteInfoToConsole();

	RegisterConsoleCommands();

	Config::Load();

	m_State->m_pGraphics = Graphics::Create( info.graphics );

	Window::Properties wndProps = info.window;
	wndProps.save = Fission::MainWindowID;
	m_State->m_pMainWindow = Window::Create( wndProps, m_State->m_pGraphics.get(), this );
}

Application::~Application() noexcept 
{
	for( auto && layer : m_State->m_vMainLayers )
		delete layer;

	Config::Save();

	delete m_State;
}

void Application::PushLayer( const char * name, ILayer * layer )
{
	m_State->m_vMainLayers.emplace_back( layer );
}

Window * Application::GetWindow()
{
	return m_State->m_pMainWindow.get();
}

Graphics * Application::GetGraphics()
{
	return m_State->m_pGraphics.get();
}

Platform::ExitCode Application::Run()
{
	m_State->m_bRunning = true;
	OnCreate();
create:
	if( m_State->m_bRecreate )
	{
	//	_Recreate();
		m_State->m_bRecreate = false;
	}

	m_State->m_UILayer.OnCreate();
	m_State->m_ConsoleLayer.OnCreate();
	m_State->m_DebugLayer.OnCreate();
	for( auto && layer : m_State->m_vMainLayers )
		layer->OnCreate();

	while( m_State->m_bRunning )
	{
		// Check to see if Window is minimized, if so wait until window is visible again
		if( m_State->m_bMinimized )
		{
			std::mutex mutex;
			std::unique_lock lock( mutex );
			m_State->m_PauseCondition.wait( lock );
		}

		m_State->m_pMainWindow->GetSwapChain()->Bind();
		m_State->m_pMainWindow->GetSwapChain()->Clear( Colors::Black );

		// Update all layers from back to top*
		for( auto && layer : m_State->m_vMainLayers )
			layer->OnUpdate();
		m_State->m_UILayer.OnUpdate();
		m_State->m_ConsoleLayer.OnUpdate();
		m_State->m_DebugLayer.OnUpdate();

		//m_State->m_pGraphics->EndFrame();
		m_State->m_pMainWindow->GetSwapChain()->Present( Graphics::GetVSync() );

		s_LastDelta = s_AppTimer.gets(); // temp

		// Graphics configuration has changed, so all resources must be created again
		if( m_State->m_bRecreate )
			goto create;
	}

	return m_State->m_ExitCode;
}

void Application::Exit( Platform::ExitCode ec )
{
	m_State->m_pMainWindow->Close();
}
