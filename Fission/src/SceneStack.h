#pragma once
#include "Fission/Core/Scene.h"
#include "Fission/Core/Application.h"

namespace Fission {

	class SceneStack
	{
	public:
		SceneStack( ScenePtr start )
		{
			m_Scenes.reserve( 16 );
			m_ActiveScene = start;
			m_Scenes.emplace_back( m_ActiveScene );
		}

		inline void OnCreate() { m_ActiveScene->OnCreate(); };

		inline void OnUpdate() {
			if( m_bSceneSwitch && m_SceneSwitchTimer.peeks() >= m_SceneSwitchCooldownDuration )
			{
				if( scene ) m_Scenes.emplace_back( scene );
				else		m_Scenes.pop_back();
				m_bSceneSwitch = false;
				m_ActiveScene = m_Scenes.back();
			}

			m_ActiveScene->OnUpdate();
		}


		// these must match with @IEventHandler, no need for virtual functions
		inline EventResult OnKeyDown( KeyDownEventArgs & args ) { return m_ActiveScene->OnKeyDown( args ); }
		inline EventResult OnKeyUp( KeyUpEventArgs & args ) { return m_ActiveScene->OnKeyUp( args ); }
		inline EventResult OnTextInput( TextInputEventArgs & args ) { return m_ActiveScene->OnTextInput( args ); }
		inline EventResult OnMouseMove( MouseMoveEventArgs & args ) { return m_ActiveScene->OnMouseMove( args ); }
		inline EventResult OnMouseLeave( MouseLeaveEventArgs & args ) { return m_ActiveScene->OnMouseLeave( args ); }
		inline EventResult OnSetCursor( SetCursorEventArgs & args ) { return m_ActiveScene->OnSetCursor( args ); }
		inline EventResult OnHide() { return m_ActiveScene->OnHide(); }
		inline EventResult OnShow() { return m_ActiveScene->OnShow(); }
		inline EventResult OnClose( CloseEventArgs & args ) { return m_ActiveScene->OnClose( args ); }


		inline void OpenScene( Scene * ptr_scene )
		{
			scene = ptr_scene;
			m_SceneSwitchTimer.reset();
			m_bSceneSwitch = true;
		}

		inline void CloseScene()
		{
			if( m_Scenes.size() == 1 ) return Application::Get()->Exit();

			scene = nullptr;
			m_SceneSwitchTimer.reset();
			m_bSceneSwitch = true;
		}

		ScenePtr front() { return m_Scenes.back(); }


	private:

#if 1
		float m_SceneSwitchCooldownDuration = 0.25f;
		simple_timer m_SceneSwitchTimer;
		bool m_bSceneSwitch = false;
#endif
		bool m_bUpdating = false;

		ScenePtr scene = nullptr;
		ScenePtr m_ActiveScene;

		SceneList m_Scenes;
	};

}