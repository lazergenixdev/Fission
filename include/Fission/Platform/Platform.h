/**
*
* @file: Platform.h
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
#ifdef _WIN32
	/* Windows x64/x86 */
#define FISSION_PLATFORM_WINDOWS
#ifdef _WIN64
	/* Windows x64  */
#define FISSION_PLATFORM_WINDOWS64
#else
	/* Windows x86 */
#define FISSION_PLATFORM_WINDOWS32
#endif
#elif defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
/* TARGET_OS_MAC exists on all the platforms
 * so we must check all of them (in this order)
 * to ensure that we're running on MAC
 * and not some other Apple platform */
#if TARGET_IPHONE_SIMULATOR == 1
#error "IOS simulator is not supported!"
#elif TARGET_OS_IPHONE == 1
#define FISSION_PLATFORM_IOS
#error "IOS is not supported!"
#elif TARGET_OS_MAC == 1
#define FISSION_PLATFORM_MACOS
#error "MacOS is not supported!"
#else
#error "Unknown Apple platform!"
#endif
/* We also have to check __ANDROID__ before __linux__
 * since android is based on the linux kernel
 * it has __linux__ defined */
#elif defined(__ANDROID__)
#define FISSION_PLATFORM_ANDROID
#error "Android is not supported!"
#elif defined(__linux__)
#define FISSION_PLATFORM_LINUX
#error "Linux is not supported!"
#else
	/* Unknown compiler/platform */
#error "Unknown platform!"
#endif // End of platform detection

#ifdef FISSION_PLATFORM_WINDOWS
#include "Windows/winapi.h"
#include <wrl/client.h>

namespace Fission::Platform
{
	using ExitCode = int;

	using WindowHandle = HWND;

	using Event = struct {
		HWND hWnd;
		UINT Msg;
		WPARAM wParam;
		LPARAM lParam;
	};

	template <typename T>
	using com_ptr = Microsoft::WRL::ComPtr<T>;
}
#endif // FISSION_PLATFORM_WINDOWS