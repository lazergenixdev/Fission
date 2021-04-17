#pragma once
#include "Fission/config.h"

namespace Fission
{

struct DisplayMode
{
	vec2i resolution;
	int refresh_rate;
};

class Monitor;

//! Public API will always use raw pointers,
//! because monitors are something that will never
//! need to be changed by the user
using MonitorPtr = Monitor *;

class Monitor
{
public:
	/*! @brief Native monitor handle type */
	using native_handle_type = Platform::MonitorHandle;

public:

	FISSION_API static list_view<MonitorPtr> GetMonitors();

	virtual const wchar_t * GetName() const = 0;

	virtual const DisplayMode * GetCurrentDisplayMode() const = 0;

	//! @brief Change the display mode that is currently set.
	virtual bool SetDisplayMode(const DisplayMode *) = 0;

	//! @brief Revert the display mode to the user's default for this monitor.
	virtual bool RevertDisplayMode() = 0;

	virtual list_view<DisplayMode> GetSupportedDisplayModes() = 0;

	virtual native_handle_type native_handle() = 0;

}; // class Fission::Monitor

} // namespace Fission
