#pragma once
#include <Fission/Core/Scene.hh>
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Application.hh>
#include <Fission/Base/Utility/Timer.h>

namespace Fission {

	struct DebugScene : public FScene
	{
		static DebugScene * Get() { static DebugScene scene; return &scene; }
		virtual void Destroy() override { (void)0; }
	};

	class SceneStack
	{
	public:
		SceneStack()
		{
			m_vScenes.reserve(5);
			m_vScenes.emplace_back( m_ActiveScene );
		}

		FISSION_FORCE_INLINE void OnCreate(FApplication * app) { m_ActiveScene->OnCreate( app ); };

		FISSION_FORCE_INLINE void OnUpdate(FApplication * app) {
			if( m_bSceneSwitch && m_SceneSwitchTimer.peeks() >= m_SceneSwitchCooldownDuration )
			{
				if( m_NextScene )
				{
					m_ActiveScene = m_NextScene;
					m_vScenes.emplace_back( m_NextScene );
					m_ActiveScene->OnCreate(app);
				}
				else 
				{ 
					// Active scene must always be available, so switch active scene before deleting
					m_ActiveScene = *( m_vScenes.end() - 2 );
					m_vScenes.back()->Destroy();
					m_vScenes.pop_back();
				}
				m_bSceneSwitch = false;
			}
			m_ActiveScene->OnUpdate();
		}

		// these must match with @IEventHandler, no need for virtual functions
		FISSION_FORCE_INLINE EventResult OnKeyDown( KeyDownEventArgs & args )       { return m_ActiveScene->OnKeyDown( args );    }
		FISSION_FORCE_INLINE EventResult OnKeyUp( KeyUpEventArgs & args )           { return m_ActiveScene->OnKeyUp( args );      }
		FISSION_FORCE_INLINE EventResult OnTextInput( TextInputEventArgs & args )   { return m_ActiveScene->OnTextInput( args );  }
		FISSION_FORCE_INLINE EventResult OnMouseMove( MouseMoveEventArgs & args )   { return m_ActiveScene->OnMouseMove( args );  }
		FISSION_FORCE_INLINE EventResult OnMouseLeave( MouseLeaveEventArgs & args ) { return m_ActiveScene->OnMouseLeave( args ); }
		FISSION_FORCE_INLINE EventResult OnSetCursor( SetCursorEventArgs & args )   { return m_ActiveScene->OnSetCursor( args );  }
		FISSION_FORCE_INLINE EventResult OnHide()                                   { return m_ActiveScene->OnHide();             }
		FISSION_FORCE_INLINE EventResult OnShow()                                   { return m_ActiveScene->OnShow();             }
		FISSION_FORCE_INLINE EventResult OnClose( CloseEventArgs & args )           { return m_ActiveScene->OnClose( args );      }

		// ptr_scene must never be nullptr
		inline void OpenScene( FScene * ptr_scene )
		{
			if( m_vScenes.size() )
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
				m_vScenes.emplace_back( ptr_scene );
				m_ActiveScene = m_vScenes.back();
			}
		}

		inline void CloseScene()
		{
			// We only have this scene and debug scene left, we should close up.
		//	if( m_vScenes.size() <= 2 ) return mApp->pEngine->Shutdown(0x45<<1);

			if( !m_bSceneSwitch )
			{
				m_NextScene = nullptr;
				m_SceneSwitchTimer.reset();
				m_bSceneSwitch = true;
			}
		}

	private:

		FScene * m_ActiveScene = DebugScene::Get();
		bool m_bSceneSwitch = false;

		float m_SceneSwitchCooldownDuration = 0.2f;
		simple_timer m_SceneSwitchTimer;
		FScene * m_NextScene = nullptr;

		std::vector<FScene *> m_vScenes;
	};

}