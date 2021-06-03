#pragma once
#include "Fission/Core/Scene.h"
#include "Fission/Base/Utility/Timer.h"
#include "Fission/Core/Application.h"

namespace Fission {

	class DefaultScene : public Scene
	{
	public:
		virtual EventResult OnKeyDown( KeyDownEventArgs & )			FISSION_EVENT_DEFAULT;
		virtual EventResult OnKeyUp( KeyUpEventArgs & )				FISSION_EVENT_DEFAULT;
		virtual EventResult OnTextInput( TextInputEventArgs & )		FISSION_EVENT_DEFAULT;
		virtual EventResult OnMouseMove( MouseMoveEventArgs & )		FISSION_EVENT_DEFAULT;
		virtual EventResult OnMouseLeave( MouseLeaveEventArgs & )	FISSION_EVENT_DEFAULT;
		virtual EventResult OnSetCursor( SetCursorEventArgs & )		FISSION_EVENT_DEFAULT;
		virtual EventResult OnHide()								FISSION_EVENT_DEFAULT;
		virtual EventResult OnShow()								FISSION_EVENT_DEFAULT;
		virtual EventResult OnClose( CloseEventArgs & )				FISSION_EVENT_DEFAULT;
	};

	class SceneStack
	{
	public:
		SceneStack( ScenePtr start )
			: m_ActiveScene(start)
		{
			m_Scenes.reserve( 16 );
			if( m_ActiveScene )
			{
				m_Scenes.emplace_back( m_ActiveScene );
			}
			else m_ActiveScene = new DefaultScene;
		}

		inline void OnCreate() { m_ActiveScene->CreateAll(); };

		inline void OnUpdate() {
			if( m_bSceneSwitch && m_SceneSwitchTimer.peeks() >= m_SceneSwitchCooldownDuration )
			{
				if( m_NextScene )
				{
					m_ActiveScene = m_NextScene;
					m_Scenes.emplace_back( m_NextScene );
					m_ActiveScene->CreateAll();
				}
				else 
				{ 
					// Active scene must always be available, so switch active scene before deleting
					m_ActiveScene = *( m_Scenes.end() - 2 );
				//	delete m_Scenes.back(); // this is a memory leak
					m_Scenes.pop_back(); 
				}
				m_bSceneSwitch = false;
			}
			m_ActiveScene->OnUpdate();
		}


		// these must match with @IEventHandler, no need for virtual functions
		inline EventResult OnKeyDown( KeyDownEventArgs & args )			{ return m_ActiveScene->OnKeyDown( args );		}
		inline EventResult OnKeyUp( KeyUpEventArgs & args )				{ return m_ActiveScene->OnKeyUp( args );		}
		inline EventResult OnTextInput( TextInputEventArgs & args )		{ return m_ActiveScene->OnTextInput( args );	}
		inline EventResult OnMouseMove( MouseMoveEventArgs & args )		{ return m_ActiveScene->OnMouseMove( args );	}
		inline EventResult OnMouseLeave( MouseLeaveEventArgs & args )	{ return m_ActiveScene->OnMouseLeave( args );	}
		inline EventResult OnSetCursor( SetCursorEventArgs & args )		{ return m_ActiveScene->OnSetCursor( args );	}
		inline EventResult OnHide()										{ return m_ActiveScene->OnHide();				}
		inline EventResult OnShow()										{ return m_ActiveScene->OnShow();				}
		inline EventResult OnClose( CloseEventArgs & args )				{ return m_ActiveScene->OnClose( args );		}

		// ptr_scene must never be nullptr
		inline void OpenScene( Scene * ptr_scene )
		{
			if( m_Scenes.size() )
			{
				if( !m_bSceneSwitch )
				{
					m_NextScene = ptr_scene;
					m_SceneSwitchTimer.reset();
					m_bSceneSwitch = true;
				}
			}
			else
			{
				m_Scenes.emplace_back( ptr_scene );
				m_ActiveScene = m_Scenes.back();
			}
		}

		inline void CloseScene()
		{
			if( m_Scenes.size() <= 1 ) return Application::Get()->Exit();

			if( !m_bSceneSwitch )
			{
				m_NextScene = nullptr;
				m_SceneSwitchTimer.reset();
				m_bSceneSwitch = true;
			}
		}

	private:

		ScenePtr m_ActiveScene;
		bool m_bSceneSwitch = false;

		float m_SceneSwitchCooldownDuration = 0.2f;
		simple_timer m_SceneSwitchTimer;
		ScenePtr m_NextScene;

		SceneList m_Scenes;
	};

}