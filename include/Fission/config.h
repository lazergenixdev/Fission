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
#include <Fission/Platform/Platform.h> /*!< Determine Target Platform */

//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\
// Configuration stuff

/* FISSION_API */
#ifdef FISSION_PLATFORM_WINDOWS
#ifdef FISSION_BUILD_DLL
#define FISSION_API __declspec(dllexport)
#else
#define FISSION_API __declspec(dllimport)
#endif // FISSION_BUILD_DLL
#else
#define FISSION_API extern
#endif // FISSION_PLATFORM_WINDOWS

/*! @brief Engine Name */
#define FISSION_ENGINE "Fission Engine"

/*! @brief Fission Engine Build String, identifying the config we built. */
#if defined(FISSION_DEBUG)
#define FISSION_BUILD_STRING "(Debug)"   /* Debug build of the engine. */
#elif defined(FISSION_RELEASE)
#define FISSION_BUILD_STRING "(Release)" /* Release build of the engine. */
#else
#define FISSION_BUILD_STRING ""          /* Distribution build of the engine. */
#endif

/*! @brief Functions that should be thread safe */
#define FISSION_THREAD_SAFE

/* FISSION_FORCE_INLINE */
#if _MSC_VER
#define FISSION_FORCE_INLINE __forceinline
#else
#define FISSION_FORCE_INLINE
#endif

//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\

/**
 * standard library includes
 */
#include <memory>
#include <utility>
#include <algorithm> /* Literally one of the most useful headers in the C++ standard lib. */
#include <functional>
#include <optional> /* I am only optionally including this header. */
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <chrono> /* "I told you not to fuck with time Morty!" */
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <thread>

//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\

/**
 * Assertions
 */
#include <cassert>
#if FISSION_DEBUG && 1
#define FISSION_ASSERT( expression, ... ) assert( expression && "" __VA_ARGS__ )
#else
#define FISSION_ASSERT( expression, ... ) ((void)0)
#endif // FISSION_DEBUG

/**
 * Macro Helpers
 */
#define FISSION_MK_STR(X) #X

//! @brief Convert the version to a single integer for version comparing.
//! @note IMPLEMENT ME (TODO:)
#define FISSION_VERSION_AS_INT(MAJ,MIN,PAT) float(MAJ ^ MIN ^ PAT)/0.0f

/**
* Important Web Address
*/
#define FISSION_Rx2 "https://youtu.be/dQw4w9WgXcQ"

//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\

/**
* TODO: Find a place for this in Base/
*/
namespace Fission {
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
