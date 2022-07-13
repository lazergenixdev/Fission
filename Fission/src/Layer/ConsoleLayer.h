#pragma once
#include <Fission/Base/String.hpp>
#include <Fission/Base/Utility/Timer.h>
#include <Fission/Core/Scene.hh>

namespace Fission {

class ConsoleLayerImpl : public IFConsoleLayer
{
public:
	virtual void OnCreate(class FApplication * app) override;
	virtual void OnUpdate(timestep dt) override;

	virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;
	virtual EventResult OnTextInput( TextInputEventArgs & args ) override;
	virtual EventResult OnResize( ResizeEventArgs & args ) override;

	virtual void Destroy() override;
private:
	void _hide();

private:
	IFRenderer2D* m_pRenderer2D;

	string m_CommandText;

	static constexpr uint32_t s_MaxCommandSize = 69;

	float extend_rate = 6.0f;
	float extend = 0.0f;

	float m_BlinkPosition = 0.0f; // show: [0 - 1); hidden: [1 - 2);
	int m_CursorPosition = 0;

	uint32_t lineOffset = 0;
	bool m_bShow = false;

	float m_width;

	Font * m_pFont = nullptr;
	float m_FontSize;

	std::vector<string> m_History;
	int m_LastHistoryIndex = 0;

	static constexpr float m_BottomPadding = 2.0f;
	static constexpr float m_LeftPadding   = 5.0f;
};

}
