/**
*
* @file: BitFlag.h
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
#include "../config.h"
#include <type_traits>

_FISSION_BASE_PUBLIC_BEGIN


#define _Fission_Declare_Enum_Operators(_Enum) \
inline constexpr auto operator|(const _Enum &_Left, const _Enum &_Right) {\
using flag_t=::Fission::utility::bit_flag_t<_Enum>;return static_cast<flag_t>(_Left)|static_cast<flag_t>(_Right); } \
inline constexpr auto operator&(const _Enum &_Left, const _Enum &_Right) {\
using flag_t=::Fission::utility::bit_flag_t<_Enum>;return static_cast<flag_t>(_Left)&static_cast<flag_t>(_Right); }


namespace utility
{

	//! @brief Get the number of bits in a type.
	template <typename _Ty>
	static constexpr auto bitsof = sizeof(_Ty)*8;

	// glorious meta-programming, very cool.
	template <int _Bits> struct _bit_flag_impl { using type = void; };

	template <> struct _bit_flag_impl< 8> { using type = ::uint8_t;  };
	template <> struct _bit_flag_impl<16> { using type = ::uint16_t; };
	template <> struct _bit_flag_impl<32> { using type = ::uint32_t; };
	template <> struct _bit_flag_impl<64> { using type = ::uint64_t; };

	//! @brief Bit flag type, templated on the number of bits available.
	template <int _Bits>
	using bit_flag = typename _bit_flag_impl<_Bits>::type;

	//! @brief Bit flag type, templated on an enum.
	template <typename _Enum>
	using bit_flag_t = std::enable_if_t<std::is_enum_v<_Enum>,typename _bit_flag_impl<bitsof<_Enum>>::type>;


	//! @brief Create a flag from the bit index.
	template <int _Bit_Index, int _Bits>
	static constexpr auto make_flag = static_cast<std::enable_if_t<( _Bit_Index < _Bits ), bit_flag<_Bits>>>( 1llu << _Bit_Index );

	//! @brief Removes the specified bit or bits from a flag variable.
	template <typename _Ty, typename _Bit_Type>
	static constexpr _Ty & remove_flag( _Ty & _Flags, const _Bit_Type & _Bit ) {
		static_assert( std::is_fundamental_v<_Ty> && (std::is_integral_v<_Bit_Type> || std::is_enum_v<_Bit_Type>) );
		return( _Flags &=~ static_cast<_Ty>(_Bit) );
	}

	//! @brief Sets the specified bit or bits from a flag variable.
	template <typename _Ty, typename _Bit_Type>
	static constexpr _Ty & set_flag( _Ty & _Flags, const _Bit_Type & _Bit ) {
		static_assert( std::is_fundamental_v<_Ty> && (std::is_integral_v<_Bit_Type> || std::is_enum_v<_Bit_Type>) );
		return( _Flags |= static_cast<_Ty>(_Bit) );
	}

} // namespace Fission::utility

_FISSION_BASE_PUBLIC_END
