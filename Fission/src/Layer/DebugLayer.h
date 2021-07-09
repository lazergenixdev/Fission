#pragma once
#include <Fission/Base/Utility/Timer.h>
#include <Fission/Core/Scene.hh>

namespace Fission {

	class DebugLayerImpl : public IFDebugLayer
	{
	public:
		DebugLayerImpl();

		void SetAppVersionString( const string & version_String );

		virtual void RegisterDrawCallback( const char * _Key, DrawCallback _Callback, void * _UserData ) override;

		void Text( const char * what );

		virtual void OnCreate(class FApplication *) override;
		virtual void OnUpdate(timestep dt) override;

		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;
		virtual EventResult OnResize( ResizeEventArgs & args ) override;

		virtual void Destroy() override;
	private:
		IFRenderer2D * pRenderer2D = nullptr;

		struct DrawCallbackData {
			DrawCallback callback;
			void * user;
		};
		std::map<std::string, DrawCallbackData> m_DrawCallbacks;

		string m_AppVersionString = "<APP VERSION> (nice version you got there)";
		std::vector<std::string> m_UserInfo;

		bool m_bShow = false;
		bool m_bShowFR = false;

		float m_width;

		float m_LastFrameTimes[64] = {};
		size_t m_FrameCount = 0;
	};

}
