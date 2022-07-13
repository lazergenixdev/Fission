/**
 * @file config.hpp
 * @author lazergenixdev@gmail.com
 * 
 *	 _______   _   _____   _____   _   _____   __    _  
 *	|  _____| | | |  ___| |  ___| | | |  _  | |  \  | | 
 *	| |___    | |  \ \     \ \    | | | | | | |   \ | | 
 *	|  ___|   | |   \ \     \ \   | | | | | | | |\ \| | 
 *	| |       | |  __\ \   __\ \  | | | |_| | | | \   | 
 *	|_|       |_| |_____| |_____| |_| |_____| |_|  \__| 
 * 
 *	MIT License
 *	
 *	Copyright (c) 2021-2022 Lazergenix
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
#pragma once
#include <Fission/Platform/Platform.h> /*!< Determine Target Platform */

// Fission namespace will be used a lot, no need 
//	for extra indentation.

#define __FISSION_BEGIN__ namespace Fission {
#define __FISSION_END__ }


//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\
// Configuration stuff

/* FISSION_API */
#ifdef FISSION_PLATFORM_WINDOWS
#	ifdef FISSION_BUILD_DLL
#		define FISSION_API __declspec(dllexport)
#	else
#		define FISSION_API __declspec(dllimport)
#	endif // FISSION_BUILD_DLL
#else
#	define FISSION_API extern
#endif // FISSION_PLATFORM_WINDOWS

/*! @brief Engine Name */
#define FISSION_ENGINE "Fission Engine"

/*! @brief Fission Engine Build String, identifying the config we built. */
#ifdef FISSION_DEBUG
#	define FISSION_BUILD_STRING "(Debug)"   /* Debug build of the engine. */
#elif defined(FISSION_RELEASE)
#	define FISSION_BUILD_STRING "(Release)" /* Release build of the engine. */
#else
#	define FISSION_BUILD_STRING ""          /* Distribution build of the engine. */
#endif

/*! @brief Functions that should be thread safe */
#define FISSION_THREAD_SAFE

#if _MSC_VER
#	define FISSION_COMPILER_MSVC 1
#else
#	error "Compiler not recognized!"
#endif

/* FISSION_FORCE_INLINE */
#if FISSION_COMPILER_MSVC
#	define FISSION_FORCE_INLINE __forceinline
#else
#	define FISSION_FORCE_INLINE
#endif

//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\

/**
* convert: Bool TO String
*/
#define FISSION_BTOS(B) (B?("True"):("False"))

/**
* Important Web Address
*/
#define FISSION_Rx2 "https://youtu.be/dQw4w9WgXcQ"

/*! @brief Engine Name */
#define ENGINE_NAME "Fission Engine"

/*! @brief Functions that should be thread safe */
#define FISSION_THREAD_SAFE

/**
 * Assertions // TODO: Don't throw an exception.
 */
#if FISSION_DEBUG && false
#define FISSION_ASSERT( EXPRESSION, ... ) if(!(EXPRESSION)) \
throw ::Fission::base::generic_error("Assertion Failed", \
::Fission::base::error_message(FISSION_THROW_LOCATION()).append("expression",#EXPRESSION).append(__VA_ARGS__) )
#else
#define FISSION_ASSERT( EXPRESSION, ... ) ((void)0)
#endif // FISSION_DEBUG
