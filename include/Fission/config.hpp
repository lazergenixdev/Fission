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

///////////////////////////////////////////////////////////////////////////////
// Macro Helpers

#define MACRO_EXPAND(X) X
#define MACRO_STRING(X) #X
#define MACRO_STRING_EXPAND(X) MACRO_STRING(X)
#define MACRO_JOIN(A,B) A ## B
#define MACRO_JOIN_EXPAND(A,B) MACRO_JOIN(A,B)
#define MACRO_PRAGMA(X) _Pragma(#X)
#define NO_DISCARD [[nodiscard]]


///////////////////////////////////////////////////////////////////////////////

// FISSION_COMPILER_XX
#if defined(__clang__)
#	define FISSION_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#   define FISSION_COMPILER_GNU
#elif defined(_MSC_VER)
#   define FISSION_COMPILER_MSVC
#else
#	error "Compiler not recognized!"
#endif

// FISSION_DISABLE_WARNING(WARNING)
#if defined(FISSION_COMPILER_MSVC)
#   define FISSION_DISABLE_WARNING(WARNINGS) MACRO_PRAGMA(warning(disable: WARNINGS))
#   define FISSION_DISABLE_ALL_WARNINGS_BEGIN MACRO_PRAGMA(warning(push, 0))
#   define FISSION_DISABLE_ALL_WARNINGS_END   MACRO_PRAGMA(warning(pop))
#elif defined(FISSION_COMPILER_CLANG) || defined(FISSION_COMPILER_GCC)
#   define FISSION_DISABLE_WARNING(WARNING) MACRO_PRAGMA(GCC diagnostic ignored WARNING)
#   define FISSION_DISABLE_ALL_WARNINGS_BEGIN \
    MACRO_PRAGMA(GCC diagnostic push) FISSION_DISABLE_WARNING("-Weverything")
#   define FISSION_DISABLE_ALL_WARNINGS_END   MACRO_PRAGMA(GCC diagnostic pop)
#endif

// FISSION_API
#define FISSION_SHARED 0
#if FISSION_SHARED
#    if FISSION_BUILD
#        define FISSION_API FISSION_SHARED_EXPORT
#    else
#        define FISSION_API FISSION_SHARED_IMPORT
#    endif
#else
#    define FISSION_API extern
#endif

#include "detect_platform.hpp"
#include <stdint.h> // Vulkan includes also this
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////
// Disable Warnings

#if   defined(FISSION_COMPILER_MSVC)

// 'bytes' bytes padding added after construct 'member_name'
	FISSION_DISABLE_WARNING(4820)

// enumerator 'identifier' in switch of enum 'enumeration'
// is not explicitly handled by a case label
	FISSION_DISABLE_WARNING(4061)

// 4625 => copy constructor
// 5026 => move constructor
// 4626 => copy operator
// 5027 => move operator    ... was implicitly deleted
	FISSION_DISABLE_WARNING(4625 5026 4626 5027)

// 'function' : unreferenced inline function has been removed
	FISSION_DISABLE_WARNING(4514)

// Compiler will insert Spectre mitigation for memory load
// if /Qspectre switch specified
	FISSION_DISABLE_WARNING(5045)

// 'operation': unsafe conversion from 'type_of_expression'
// to 'type_required'
	FISSION_DISABLE_WARNING(4191)

// 'derived class' : default constructor was implicitly defined as deleted
	FISSION_DISABLE_WARNING(4623)

// nameless struct/union
	FISSION_DISABLE_WARNING(4201)

#elif defined(FISSION_COMPILER_CLANG)

// It's reserved??.. Who asked??????
	FISSION_DISABLE_WARNING("-Wreserved-macro-identifier")

	FISSION_DISABLE_WARNING("-Wc++98-compat")
	FISSION_DISABLE_WARNING("-Wc++98-compat-pedantic")

// Nearly impossible to silence this warning,
// great job GCC, I'm so proud of you.
	FISSION_DISABLE_WARNING("-Wunsafe-buffer-usage")

// It's called "C-style cast", go fuck yourself
	FISSION_DISABLE_WARNING("-Wold-style-cast")

// ????
	FISSION_DISABLE_WARNING("-Wc++20-extensions")

	FISSION_DISABLE_WARNING("-Wnested-anon-types")

#endif


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

// Fission namespace (fs) will be used a lot, no need
//	for extra indentation.
#define __FISSION_BEGIN__ namespace fs {
#define __FISSION_END__ }

/// Convert bool value to "True" or "False"
#define FS_BTF(B) ((B)?"True":"False")

/// Convert bool value to "Yes" or "No"
#define FS_BYN(B) ((B)?"Yes":"No")

/// Very important web address
#define FS_IMPORTANT_LINK "https://youtu.be/dQw4w9WgXcQ"

#define FS_PI  (3.1415926535897932384626433)
#define FS_TAU (6.2831853071795864769252867)

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

#define FMT_HEADER_ONLY 1


__FISSION_BEGIN__

///////////////////////////////////////////////////////////////////////////////
// Types

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

/// This is only for seeing the strings correctly in the debugger,
///    literally no other reason for this to be here :)
#if defined(__cpp_char8_t)
using c8  = char8_t;
using c16 = char16_t;
using c32 = char32_t;
#else
using c8  = u8;
using c16 = u16;
using c32 = u32;
#endif


///////////////////////////////////////////////////////////////////////////////
// Meta Templates

template <typename>
static constexpr bool always_false = false; // c++ really do be this dumb...


// Base
template <int, typename...>
struct _type_at { using type = void; };

// Single Type
template <int i, typename T>
struct _type_at<i, T> { using type = T; };

// Two or more Types
template <int i, typename T, typename...Rest>
struct _type_at<i, T, Rest...> {
	using type = std::conditional_t<i <= 0, T, typename _type_at<i - 1, Rest...>::type>;
};

template <int i, typename...T>
using type_at = typename _type_at<i, T...>::type;


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

template <typename...T>
static constexpr u32 size_of = _size_of_n<sizeof...(T), T...>::value;

__FISSION_END__

/**
 *	MIT License
 *
 *	Copyright (c) 2021-2025 lazergenixdev
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
