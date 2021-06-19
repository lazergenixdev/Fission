#pragma once
#include <Fission/Base/Utility/Timer.h>
#include <Fission/Core/Scene.hh>

namespace Fission {

	class DebugLayerImpl : public IFDebugLayer
	{
	public:
		DebugLayerImpl();

		virtual void RegisterDrawCallback( const char * _Key, DrawCallback _Callback, void * _UserData ) override;

		void Push( const char * name );

		void Pop();

		void Text( const char * what );

		virtual void OnCreate(class FApplication *) override;
		virtual void OnUpdate() override;

		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;

		virtual void Destroy() override;
	private:
		IFRenderer2D * pRenderer2D = nullptr;

		struct DrawCallbackData {
			DrawCallback callback;
			void * user;
		};
		std::map<std::string, DrawCallbackData> m_DrawCallbacks;

		std::map<std::string, std::vector<std::string>> m_InfoMap;
		std::vector<std::string> * m_CurrentInfo = nullptr;

		bool m_bShow = false;
		bool m_bShowFR = false;

		float m_LastFrameTimes[64] = {};
		size_t m_FrameCount = 0;

		simple_timer t;
	};

}
