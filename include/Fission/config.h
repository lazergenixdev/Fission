/**
*
* @file: config.h
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

/*! @brief Engine Name */
#define FISSION_ENGINE "Fission"

/*! @brief Functions that should be thread safe */
#define FISSION_THREAD_SAFE

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
namespace Fission {
	using namespace lazer;
}

/**
 * Pointer Types
 */
namespace Fission {

	using namespace lazer;

	template <typename T>
	using ref = std::shared_ptr<T>;
	template <typename T>
	using scoped = std::unique_ptr<T>;

	template <typename T, typename...Args>
	static constexpr ref<T> CreateRef( Args&&...args )
	{
		return std::make_shared<T>( std::forward<Args>( args ) ... );
	}
	template <typename T, typename...Args>
	static constexpr scoped<T> CreateScoped( Args&&...args )
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
#define IMGUI_DISABLE //  Disable everything ImGui: all headers and source files will be empty.
#endif
#endif

#define IMGUI_API FISSION_API
