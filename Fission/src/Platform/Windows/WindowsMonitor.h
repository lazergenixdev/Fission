#pragma once
#include <Fission/Core/Monitor.hh>

namespace Fission::Platform {

	extern void EnumMonitors();

	class WindowsMonitor : public Monitor
	{
	public:
		WindowsMonitor( HMONITOR hMonitor, const std::wstring & Name );

		virtual const wchar_t * GetName() const override;

		virtual const DisplayMode * GetCurrentDisplayMode() const override;

		virtual list_view<DisplayMode> GetSupportedDisplayModes() override;

		virtual bool SetDisplayMode( const DisplayMode * ) override;

		virtual bool RevertDisplayMode() override;

		virtual native_handle_type native_handle() override;

	private:

		DisplayMode * FindClosestDisplayMode( const DisplayMode & mode );

	private:

		std::wstring m_Name = L"<Monitor Name>";
		std::wstring m_DeviceName = L"<Device Name>";
		std::vector<DisplayMode> m_SupportedModes;
		const DisplayMode * m_pCurrentMode;
		HMONITOR m_hMonitor;

	};

}
