/**
*
* @file: Monitor.h
* @author: lazergenixdev@gmail.com
*
*
* This file is provided under the MIT License:
*
* Copyright (c) 2021 Lazergenix Software
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#pragma once
#include <Fission/config.h>
#include <Fission/Base/Size.h>

namespace Fission
{

struct DisplayMode
{
	base::size resolution;
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

//	virtual int GetIndex() const = 0;

	//! @brief Change the display mode that is currently set.
	virtual bool SetDisplayMode(const DisplayMode *) = 0;

	//! @brief Revert the display mode to the user's default for this monitor.
	virtual bool RevertDisplayMode() = 0;

	virtual list_view<DisplayMode> GetSupportedDisplayModes() = 0;

	virtual native_handle_type native_handle() = 0;

}; // class Fission::Monitor

} // namespace Fission
