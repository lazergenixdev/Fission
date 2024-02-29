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
// I am lazy, sorry :(
// https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/PlatformDetection.h
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
#else
	/* Unknown compiler/platform */
	#error "Unknown platform!"
#endif // End of platform detection

#include "inttypes.h"

#if defined(FISSION_PLATFORM_WINDOWS)
#define FISSION_MAIN wWinMain
#define FISSION_MAIN_FUNCTION() int WINAPI wWinMain( \
_In_ HINSTANCE hInstance,                            \
_In_opt_ HINSTANCE hPrevInstance,                    \
_In_ LPWSTR lpCmdLine,                               \
_In_ int nShowCmd                                    \
)
#define FISSION_MAIN_ARGS
#define FISSION_SHARED_EXPORT __declspec(dllexport)
#define FISSION_SHARED_IMPORT __declspec(dllimport)
#elif defined(FISSION_PLATFORM_LINUX)
#define FISSION_MAIN main
#define FISSION_MAIN_FUNCTION() int main(int argc, char* argv[])
#define FISSION_MAIN_ARGS argc, argv
#define FISSION_SHARED_EXPORT extern
#define FISSION_SHARED_IMPORT extern
#endif

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
