/**
 *	______________              _____
 *	___  ____/__(_)________________(_)____________
 *	__  /_   __  /__  ___/_  ___/_  /_  __ \_  __ \
 *	_  __/   _  / _(__  )_(__  )_  / / /_/ /  / / /
 *	/_/      /_/  /____/ /____/ /_/  \____//_/ /_/
 *
 *
 * @Author:       lazergenixdev@gmail.com
 * @Development:  (https://github.com/lazergenixdev/Fission)
 * @License:      MIT (see end of file)
 */
#pragma once
#ifdef _WIN32
	/* Windows x64/x86 */
	#define FISSION_PLATFORM_WINDOWS 1
	#ifdef _WIN64
		/* Windows x64  */
		#define FISSION_PLATFORM_WINDOWS64 1
	#else
		/* Windows x86 */
		#define FISSION_PLATFORM_WINDOWS32 1
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
		#define FISSION_PLATFORM_IOS 1
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define FISSION_PLATFORM_MACOS 1
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
/* We also have to check __ANDROID__ before __linux__
 * since android is based on the linux kernel
 * it has __linux__ defined */
#elif defined(__ANDROID__)
	#define FISSION_PLATFORM_ANDROID 1
	#error "Android is not supported!"
#elif defined(__linux__)
	#define FISSION_PLATFORM_LINUX 1
	#error "Linux is not supported!"
#else
	/* Unknown compiler/platform */
	#error "Unknown platform!"
#endif // End of platform detection

#ifdef FISSION_PLATFORM_WINDOWS
	#include <Fission/Platform/Windows/winapi.h>

	//! @note Our main will be called only from WinMain,
	//!	       thus we just want to inline the code
	#define FISSION_MAIN_EXPORT(RET) __forceinline RET

	namespace Fission
	{
		using platform_char = wchar_t;
	}

	namespace Fission::Platform
	{
		using ExitCode = int;

		using WindowHandle = HWND;
		using MonitorHandle = HMONITOR;

		using Event = struct {
			HWND hWnd;
			UINT Msg;
			WPARAM wParam;
			LPARAM lParam;
		};
	}

#elif FISSION_PLATFORM_ANDROID

	#define FISSION_MAIN_EXPORT(RET) extern "C" RET

	namespace Fission::Platform
	{
		using ExitCode = int;

		using WindowHandle = void*;
		using MonitorHandle = void*;

		using Event = struct { int unused; };
	}

#endif // FISSION_PLATFORM_*

/**
 *	MIT License
 *
 *	Copyright (c) 2021-2023 lazergenixdev
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */