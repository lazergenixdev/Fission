//
//	while( m_State->bRunning )
//	{
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