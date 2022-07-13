/**
 * @file Types.hpp
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
#include <Fission/config.hpp>

#include <cstdint>
#include <concepts>
#include <type_traits>
#include <functional>

#define __FISSION_BASE_ALIASES__(BASE,NAME) \
using NAME ## i8  = BASE<i8>; \
using NAME ## u8  = BASE<u8>; \
using NAME ## i16 = BASE<i16>; \
using NAME ## u16 = BASE<u16>; \
using NAME ## i32 = BASE<i32>; \
using NAME ## u32 = BASE<u32>; \
using NAME ## i64 = BASE<i64>; \
using NAME ## u64 = BASE<u64>; \
using NAME ## f32 = BASE<f32>; \
using NAME ## f64 = BASE<f64>


__FISSION_BEGIN__

using i8  =   int8_t;
using u8  =  uint8_t;
using i16 =  int16_t;
using u16 = uint16_t;
using i32 =  int32_t;
using u32 = uint32_t;
using i64 =  int64_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using chr = char32_t;

using byte = uint8_t;


namespace util
{
	// useful concepts
	using std::regular,
			std::semiregular,
			std::convertible_to,
			std::same_as;

	// useful types
	using std::true_type, std::false_type;

	using empty_t = struct {};


	template <typename T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template <typename T>
	concept pointer = std::is_pointer_v<T>;

	// ? remove ?
	// not the best test for allocator, but eh..
	template <class T>
	concept allocator = std::is_fundamental_v<typename std::allocator_traits<T>::value_type>;

	template <class _String_Type, typename _Elem>
	concept string_view = requires( _String_Type sv ) {
		{ sv.data() } -> convertible_to<_Elem*>;
		{ sv.size() } -> convertible_to<size_t>;
	};
	//	same_as<typename std::function<decltype(S::data)>::result_type, std::add_pointer_t<T>> &&

	template <typename T>
	static constexpr bool is_char_v = std::is_integral_v<T>;

	template <typename A, typename B>
	static constexpr bool is_same_size_v = sizeof(A) == sizeof(B);
	
	template <typename _Ty, typename..._Types>
	static constexpr bool is_any_of_v = ( ... || std::is_same_v<_Ty, _Types> );

	template <typename T, typename...F>
	struct is_any_of : std::bool_constant<is_any_of_v<T,F...>> {};

	// [x, y]
	template <typename V>
	concept vector2 =
		semiregular<V> &&
		arithmetic<decltype(V::x)> &&
		same_as<decltype(V::x), decltype(V::y)> &&
		!requires(V v) { v.z; }; // not a vector3 or vector4
	
	// [x, y, z]
	template <typename V>
	concept vector3 =
		semiregular<V> &&
		arithmetic<decltype(V::x)> &&
		same_as<decltype(V::x), decltype(V::y)> &&
		same_as<decltype(V::x), decltype(V::z)> &&
		!requires(V v) { v.w; };  // not a vector4
	
	// [x, y, z, w]
	template <typename V>
	concept vector4 =
		semiregular<V> &&
		arithmetic<decltype(V::x)> &&
		same_as<decltype(V::x), decltype(V::y)> &&
		same_as<decltype(V::x), decltype(V::z)> &&
		same_as<decltype(V::x), decltype(V::w)>;


	template <class T>
	concept chr_encoding = 
		same_as<std::remove_cv_t<decltype(T::name)>,char const *> &&
		is_char_v<typename T::value_type> &&
		is_char_v<typename T::cstr_type> &&
		is_same_size_v<typename T::value_type, typename T::cstr_type>;

} // namespace Fission::util

template <typename T>
using allocator = std::allocator<T>;


/* Character Encodings: */

// Would love to use template magic here, but unfortunatly I cannot :(
#define FISSION_MAKE_CHR_ENCODING(ENC,NAME,CHAR_T,CSTR_T)\
struct ENC { static constexpr auto name = NAME; using value_type=CHAR_T; using cstr_type=CSTR_T; }

FISSION_MAKE_CHR_ENCODING( utf8,  "UTF-8",  char8_t,  char     );
FISSION_MAKE_CHR_ENCODING( utf16, "UTF-16", char16_t, wchar_t  );
FISSION_MAKE_CHR_ENCODING( utf32, "UTF-32", char32_t, uint32_t );
FISSION_MAKE_CHR_ENCODING( ascii, "ASCII",  char,     char     );

using default_encoding = utf8;


/* Data Encodings: */

struct base64url
{
	static constexpr auto name = "Base64-URL";
	static constexpr auto alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890-_";
};

__FISSION_END__
