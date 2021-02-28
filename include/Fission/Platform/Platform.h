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

	template <typename T>
	using com_ptr = Microsoft::WRL::ComPtr<T>;
}
#endif // FISSION_PLATFORM_WINDOWS