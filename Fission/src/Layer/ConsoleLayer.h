#pragma once
#include <Fission/Base/String.h>
#include <Fission/Base/Utility/Timer.h>
#include <Fission/Core/Scene.hh>

namespace Fission {

class ConsoleLayerImpl : public IFConsoleLayer
{
public:
	virtual void OnCreate(class FApplication * app) override;
	virtual void OnUpdate() override;

	virtual EventResult OnKeyDown( KeyDownEventArgs & args ) override;
	virtual EventResult OnTextInput( TextInputEventArgs & args ) override;

	virtual void Destroy() override;
private:
	void _hide();

private:
	IFRenderer2D* m_pRenderer2D;

	string m_CommandText;

	static constexpr uint32_t s_MaxCommandSize = 69;

	float extend_rate = 6.0f;
	float extend = 0.0f;

	uint32_t lineOffset = 0;
	bool m_bShow = false;

	Font * m_pFont = nullptr;
	float m_FontSize;
	float m_BottomPadding = 2.0f;

	simple_timer t;
};

}
