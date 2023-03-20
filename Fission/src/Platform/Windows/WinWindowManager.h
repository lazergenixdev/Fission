#pragma once
#include "../WindowManager.h"

namespace Fission::Platform
{
	struct GlobalWindowInfo
	{
		const wchar_t *  WindowClassName;
		HINSTANCE        hInstance;
		Graphics *       pGraphics;
	};

	class WindowsWindowManager : public WindowManager
	{
	public:
		constexpr WindowsWindowManager() noexcept = default;

		virtual void Initialize() override;

		virtual void SetGraphics( Graphics * pGraphics ) override;

		virtual void CreateWindow( const Window::CreateInfo * pInfo, Window ** ppWindow ) override;

		virtual void Destroy() override;

	private:
		GlobalWindowInfo m_Info = {
			L"wc_Fission",
			nullptr,
			nullptr,
		};
	};

} // namespace Fission::Platform