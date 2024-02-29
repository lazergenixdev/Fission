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
#include <Fission/PlatformConfig.hpp> /*!< Determine Target Platform */
#include <utility>
#include <cstring>

// Fission namespace (fs) will be used a lot, no need 
//	for extra indentation.

#define __FISSION_BEGIN__ namespace fs {
#define __FISSION_END__ }


//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\
// Configuration stuff

/* FISSION_API */
#define FISSION_SHARED 0
#if FISSION_SHARED
#    if FISSION_BUILD
#        define FISSION_API FISSION_SHARED_EXPORT
#    else
#        define FISSION_API FISSION_SHARED_IMPORT
#    endif
#else
#    define FISSION_API
#endif

/*! @brief Fission Engine Build String, identifying the config we built. */
#ifdef FISSION_DEBUG
#	define FISSION_BUILD_STRING "(Debug)"   /* Debug build of the engine. */
#elif defined(FISSION_RELEASE)
#	define FISSION_BUILD_STRING "(Release)" /* Release build of the engine. */
#else
#	define FISSION_BUILD_STRING ""          /* Distribution build of the engine. */
#endif

#if defined(__clang__)
#	define FISSION_COMPILER_CLANG 1
#elif defined(__GNUC__) || defined(__GNUG__)
#   define FISSION_COMPILER_GNU 1
#elif defined(_MSC_VER)
#   define FISSION_COMPILER_MSVC 1
#else
#	error "Compiler not recognized!"
#endif

//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\||//\\//\\

#define FS_EXPAND(X) X
#define FS_CAT(A,B) A ## B
#define FS_CAT2(A,B) FS_CAT(A,B)

/*! convert Bool to True/False */
#define FS_BTF(B) ((B)?"True":"False")

/*! convert Bool to Yes/No */
#define FS_BYN(B) (B)?"Yes":"No")

/*! again, why is this not built into cpp???? */
#define FS_FOR(COUNT) for (std::remove_const_t<decltype(COUNT)> i = 0; i < (COUNT); ++i)

/* Important Web Address */
#define FS_IMPORTANT_LINK "https://youtu.be/dQw4w9WgXcQ"

#define FS_PI  (3.1415926535897932384626433)
#define FS_TAU (6.2831853071795864769252867)

// @TODO: this should not be here
#if defined(FISSION_DEBUG) && defined(FISSION_PLATFORM_WINDOWS)
#	define FS_debug_print(STRING) OutputDebugStringA(STRING)
#	define FS_debug_printf(FORMAT, ...) \
{ char b[128] = {}; snprintf(b, sizeof(b), FORMAT, __VA_ARGS__); OutputDebugStringA(b); } (void)0
#elif defined(FISSION_PLATFORM_LINUX)
#   define FS_debug_print(STRING) printf("%s", STRING)
#   define FS_debug_printf(FORMAT, ...) printf(FORMAT, __VA_ARGS__)
#else
#	define FS_debug_print(STRING)       (void)0
#	define FS_debug_printf(FORMAT, ...) (void)0
#endif

__FISSION_BEGIN__

// PlatformConfig.hpp is required to include "inttypes.h"

using byte = uint8_t;
using u8   = uint8_t;
using u16  = uint16_t;
using u32  = uint32_t;
using u64  = uint64_t;
using s8   = int8_t;
using s16  = int16_t;
using s32  = int32_t;
using s64  = int64_t;
using f32  = float;
using f64  = double;

// This is only for seeing the strings correctly in the debugger,
//    literally no other reason for this to be here :)
using c8  = char8_t;
using c16 = char16_t;
using c32 = char32_t;

template <typename T, typename F>
inline constexpr T lerp(T const& left, T const& right, F x) {
	return left * ((F)1 - x) + right * x;
}

// min/max from std are kinda trash ngl
template <typename _Type, typename _Convertable_To_Type>
inline constexpr _Type max(_Type a, _Convertable_To_Type b) noexcept {
	auto _b = static_cast<_Type>(b);
	if (a > _b) return a;
	return _b;
}
template <typename _Type, typename _Convertable_To_Type>
inline constexpr _Type min(_Type a, _Convertable_To_Type b) noexcept {
	auto _b = static_cast<_Type>(b);
	if (a < _b) return a;
	return _b;
}

#define FS_KILOBYTES(x) (            (x) * (::fs::u64)(1024))
#define FS_MEGABYTES(x) (FS_KILOBYTES(x) * (::fs::u64)(1024))
#define FS_GIGABYTES(x) (FS_MEGABYTES(x) * (::fs::u64)(1024))
#define FS_TERABYTES(x) (FS_GIGABYTES(x) * (::fs::u64)(1024))

#define FISSION_X_LETTERS        \
X(A)X(B)X(C)X(D)X(E)X(F)X(G)X(H) \
X(I)X(J)X(K)X(L)X(M)X(N)X(O)X(P) \
X(Q)X(R)X(S)X(T)X(U)X(V)X(W)X(X) \
X(Y)X(Z)

#define FISSION_X_BASE10 \
X(0)X(1)X(2)X(3)X(4)X(5)X(6)X(7)X(8)X(9)

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
// Template Magic: you are wrong if you think C++ couldn't get any more complicated

template <typename> static constexpr bool always_false = false; // c++ really do be this dumb...

// True Case
template <bool b, typename L, typename R>
struct _if_t { using type = L; };

// False Case
template <typename L, typename R>
struct _if_t<false, L, R> { using type = R; };

template <bool b, typename L, typename R>
using if_t = _if_t<b, L, R>::type;


// Base
template <int, typename...>
struct _type_at { using type = void; };

// Two or more Types
template <int i, typename T, typename...Rest>
struct _type_at<i, T, Rest...> { using type = if_t<i <= 0, T, typename _type_at<i - 1, Rest...>::type>; };

// Single Type
template <int i, typename T>
struct _type_at<i, T> { using type = T; };

template <int i, typename...T>
using type_at = _type_at<i, T...>::type;


// Base
template <int, typename...>
struct _size_of_n {
	static constexpr u32 value = 0u;
};

// Two or more Types
template <int i, typename T, typename...Rest>
struct _size_of_n<i, T, Rest...> {
	static constexpr u32 value = (i > 0) ? (sizeof(T) + _size_of_n<i - 1, Rest...>::value) : 0u;
};

// Single Type
template <int i, typename T>
struct _size_of_n<i, T> {
	static constexpr u32 value = (i > 0) ? sizeof(T) : 0u;
};

template <int i, typename...T>
static constexpr u32 size_of_n = _size_of_n<i, T...>::value;

//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

__FISSION_END__

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
