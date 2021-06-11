//#pragma once
//#include "Application.h"
//#include "Fission/Core/Console.h"
//#include "Fission/Core/Configuration.h"
//#include <Fission/Base/Exception.h>
//
//using namespace Fission;
//
//static Application * s_pInstance = nullptr;
//
//static float s_LastDelta = 0.0f;
//static simple_timer s_AppTimer;
//
//Application * ILayer::GetApp() {
//	return s_pInstance;
//}
//
//
//static void RegisterConsoleCommands()
//{
//	Console::RegisterCommand(
//		"exit",
//		[] ( string ) -> string {
//			Application::Get()->Exit();
//			return "exiting ..";
//		}
//	);
//	Console::RegisterCommand(
//		"vsync",
//		[] ( string s ) -> string {
//		if( s == "on" )
//		{
//			Graphics::SetVSync( vsync_On );
//			return "vsync turned on";
//		}
//		else if( s == "off" )
//		{
//			Graphics::SetVSync( vsync_Off );
//			return "vsync turned off";
//		}
//		else
//			return "incorrect format";
//		return {};
//		}
//	);
//}
//
//Application * Application::Get() {
//	return s_pInstance;
//}
//
//Application::Application( const CreateInfo & info ) 
//	: m_State( new ApplicationState(info) )
//{
//	if( s_pInstance )
//		throw std::logic_error( "more than one instance? interesting, but no" );
//
//	s_pInstance = this;
//
//	Console::WriteLine( Colors::Aqua, "Application started!" );
//
////	System::WriteInfoToConsole();
//
//	RegisterConsoleCommands();
//
//	Config::Load();
//
//	m_State->pGraphics = Graphics::Create( info.graphics );
//
//	Window::Properties wndProps = info.window;
//	wndProps.save = Fission::MainWindowID;
//	m_State->pMainWindow = Window::Create( wndProps, m_State->pGraphics.get(), this );
//}
//
//Application::~Application() noexcept 
//{
//	Config::Save();
//
//	delete m_State;
//}
//
//void Application::PushLayer( const char * name, ILayer * layer )
//{
////	m_State->m_vMainLayers.emplace_back( layer );
//}
//
//void Application::PushScene( const char * _Name, Scene * _Ptr_Scene )
//{
//	(void)_Name;
//#ifdef FISSION_DEBUG
//	if( !_Ptr_Scene )
//		throw std::logic_error( "Scene cannot be nullptr!" );
//#endif
//	m_State->SceneStack.OpenScene( _Ptr_Scene );
//}
//void Application::CloseScene()
//{
//	m_State->SceneStack.CloseScene();
//}
//
//Window * Application::GetWindow()
//{
//	return m_State->pMainWindow.get();
//}
//
//Graphics * Application::GetGraphics()
//{
//	return m_State->pGraphics.get();
//}
//
//Platform::ExitCode Application::Run()
//{
//	m_State->bRunning = true;
//	OnCreate();
//create:
//	if( m_State->bRecreate )
//	{
//	//	_Recreate();
//		m_State->bRecreate = false;
//	}
//
//	m_State->DebugLayer.OnCreate();
//	m_State->ConsoleLayer.OnCreate();
//	m_State->UILayer.OnCreate();
//	m_State->SceneStack.OnCreate();
//
//	while( m_State->bRunning )
//	{
//		// Check to see if Window is minimized, if so wait until window is visible again
//		if( m_State->bMinimized )
//		{
//			std::mutex mutex;
//			std::unique_lock lock( mutex );
//			m_State->PauseCondition.wait( lock );
//		}
//
//		m_State->pMainWindow->GetSwapChain()->Bind();
//		m_State->pMainWindow->GetSwapChain()->Clear( color(0.0f, 0.04f, 0.07f) );
//
//		m_State->SceneStack.OnUpdate();
//		m_State->UILayer.OnUpdate();
//		m_State->ConsoleLayer.OnUpdate();
//		m_State->DebugLayer.OnUpdate();
//		
//		m_State->pMainWindow->GetSwapChain()->Present( Graphics::GetVSync() );
//
//		s_LastDelta = s_AppTimer.gets(); // temp
//
//		// Graphics configuration has changed, so all resources must be created again
//		if( m_State->bRecreate ) FISSION_THROW_NOT_IMPLEMENTED()
//			
//	}
//
//	return m_State->ExitCode;
//}
//
//void Application::Exit( Platform::ExitCode ec )
//{
//	// the handler for the main window closing will tell the main loop to break.
//	m_State->pMainWindow->Close();
//}
