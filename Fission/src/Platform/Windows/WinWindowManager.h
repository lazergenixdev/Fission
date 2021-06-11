#pragma once
#include "../WindowManager.h"

namespace Fission::Platform
{
	struct GlobalWindowInfo
	{
		const wchar_t *  WindowClassName;
		HINSTANCE        hInstance;
		IFGraphics *     pGraphics;
	};

	class WindowsWindowManager : public WindowManager
	{
	public:
		constexpr WindowsWindowManager() noexcept = default;

		virtual void Initialize() override;

		virtual void SetGraphics( IFGraphics * pGraphics ) override;

		virtual void CreateWindow( const IFWindow::CreateInfo * pInfo, IFWindow ** ppWindow ) override;

		virtual void Destroy() override;

	private:
		GlobalWindowInfo m_Info = {
			L"wc_Fission",
			nullptr,
			nullptr,
		};
	};

} // namespace Fission::Platform