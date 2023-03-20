#pragma once
#include <Fission/Base/Time.hpp>
#include <Fission/Core/Scene.hh>
#include <map>

namespace Fission {

	class DebugLayerImpl : public DebugLayer
	{
	public:
		DebugLayerImpl();

		void SetAppVersionString( const string & version_String );

		virtual void RegisterDrawCallback( const char * _Key, DrawCallback _Callback, void * _UserData ) override;

		void Text( const char * what );

		virtual void OnCreate(class Application *) override;
		virtual void OnUpdate(timestep dt) override;

		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;
		virtual EventResult OnResize( ResizeEventArgs & args ) override;

		virtual void Destroy() override;
	private:
		Renderer2D * pRenderer2D = nullptr;

		struct DrawCallbackData {
			DrawCallback callback;
			void * user;
		};
		std::map<std::string, DrawCallbackData> m_DrawCallbacks;

		string m_AppVersionString;
		std::vector<std::string> m_UserInfo;

		bool m_bShow = false;
		bool m_bShowFR = false;

		float m_width;

		float m_LastFrameTimes[64] = {};
		size_t m_FrameCount = 0;
	};

}
