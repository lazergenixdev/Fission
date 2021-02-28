#pragma once
#include "Application.h"
#include "LazerEngine/Core/Console.h"
#include "LazerEngine/Core/Configuration.h"

using namespace lazer;

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
			Application::Get()->GetGraphics()->SetVSync( true );
			return L"vsync turned on";
		}
		else if( s == L"off" )
		{
			Application::Get()->GetGraphics()->SetVSync( false );
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

	Configuration::Load();

	Window::Properties wndProps = info.wndProps;
	wndProps.flags = wndProps.flags | Window::Flags::IsMainWindow;
	m_State->m_pMainWindow = Window::Create( wndProps, this );

	m_State->m_pGraphics = Graphics::Create( m_State->m_pMainWindow.get(), info.gAPI, info.resolution );
}

Application::~Application() noexcept 
{
	for( auto && layer : m_State->m_vMainLayers )
		delete layer;

	Configuration::Save();
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

DebugLayer * Application::GetDebugLayer()
{
	return &m_State->m_DebugLayer;
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

	m_State->m_DebugLayer.OnCreate();
	m_State->m_UILayer.OnCreate();
	m_State->m_ConsoleLayer.OnCreate();
	for( auto && layer : m_State->m_vMainLayers )
		layer->OnCreate();

	while( m_State->m_bRunning )
	{
		// Check to see if Window is minimized, if so wait until window is visible again
		if( m_State->m_bMinimized )
		{
			std::mutex mutex;
			std::unique_lock lock( mutex );
			m_State->m_MinimizeCondition.wait( lock );
		}

		m_State->m_pGraphics->BeginFrame();

		// Update all layers from back to top*
		for( auto && layer : m_State->m_vMainLayers )
			layer->OnUpdate();
		m_State->m_UILayer.OnUpdate();
		m_State->m_ConsoleLayer.OnUpdate();
		m_State->m_DebugLayer.OnUpdate();

		m_State->m_pGraphics->EndFrame();
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

float FrameTimer::GetLastDelta() {
	return s_LastDelta;
}
