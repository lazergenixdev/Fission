#pragma once
#include "LazerEngine/Core/Layer.h"

namespace lazer {

	class DebugLayerImpl : public DebugLayer
	{
	public:
		DebugLayerImpl();

		virtual void RegisterDrawCallback( const char * _Key, DebugDrawCallback _Callback ) override;

		void Push( const char * name );

		void Pop();

		void Text( const char * what ) ;

		virtual void OnCreate();
		virtual void OnUpdate();

		virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;
	private:
		std::unique_ptr<Renderer2D> m_pRenderer2D;

		std::map<std::string, DebugDrawCallback> m_DrawCallbacks;

		std::map<std::string, std::vector<std::string>> m_InfoMap;
		std::vector<std::string> * m_CurrentInfo = nullptr;

		bool m_bShow = false;
		bool m_bShowFR = false;

		float m_LastFrameTimes[64];
		size_t m_FrameCount = 0;

		simple_timer t;
	};

}
