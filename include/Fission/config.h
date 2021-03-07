#pragma once
#include "Platform/Platform.h" /*!< Determine Target Platform */

#if defined(FISSION_PLATFORM_WINDOWS)
#ifdef FISSION_BUILD_DLL
#define FISSION_API __declspec(dllexport)
#else
#define FISSION_API __declspec(dllimport)
#endif // FISSION_BUILD_DLL
#else
#define FISSION_API extern
#endif

#define FISSION_ENGINE "Fission" /*!< Engine Name */

#define FISSION_THREAD_SAFE /*!< Functions that should be thread safe */

/**
 * standard library includes
 */
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unordered_map>
#include <map>
#include <filesystem>

/**
 * External helper library
 */
#include "lazer/lazer.h"

/**
 * convenient type aliases 
 */
namespace Fission {

	using namespace lazer;

	template <typename T>
	using shared_ptr = std::shared_ptr<T>;
	template <typename T>
	using scoped_ptr = std::unique_ptr<T>;

	template <typename T, typename...Args>
	static constexpr shared_ptr<T> CreateShared( Args&&...args )
	{
		return std::make_shared<T>( std::forward<Args>( args ) ... );
	}
	template <typename T, typename...Args>
	static constexpr scoped_ptr<T> CreateScoped( Args&&...args )
	{
		return std::make_unique<T>( std::forward<Args>( args ) ... );
	}

}

/**
 * assertions
 */
#include <cassert>
#ifdef FISSION_DEBUG
#define FISSION_ASSERT( expression, ... ) assert( expression && "" __VA_ARGS__ )
#else
#define FISSION_ASSERT( expression, ... ) ((void)0)
#endif // FISSION_DEBUG

/**
 * macro helpers
 */
#define FISSION_MK_STR(X) #X
#define FISSION_MK_WSTR(X) L#X
#define FISSION_MK_WIDE(X) L##X

/**
 * ImGui
 */
#ifndef FISSION_IMGUI_ENABLE // ImGui only available on debug and release builds by default
#if defined(FISSION_DIST)
#define IMGUI_DISABLE //  Disable everything: all headers and source files will be empty.
#endif
#endif

#define IMGUI_API FISSION_API
