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

/**
* Important Web Address
*/
#define FISSION_RR "https://www.youtube.com/watch?v=dQw4w9WgXcQ"

/**
* Stuff that haven't been added to 'lazerlib' yet
*/

namespace lazer {
	template <typename T>
	class list_view
	{
	public:
		using iterator = T *;
		using const_iterator = const T *;

		list_view( iterator begin, iterator end ) : m_pBegin( begin ), m_pEnd( end ), m_Size( end - begin ) {}

		const T & operator[]( int index ) const { return m_pBegin[index]; }
		T & operator[]( int index ) { return m_pBegin[index]; }

		iterator begin() { return m_pBegin; }
		iterator end() { return m_pEnd; }

		const_iterator begin() const { return m_pBegin; }
		const_iterator end() const { return m_pEnd; }

		size_t size() const { return m_Size; }
	private:
		iterator m_pBegin;
		iterator m_pEnd;
		size_t m_Size;
	};
}
