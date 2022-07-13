/**
 * @file Size.hpp
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
#include <Fission/Base/Types.hpp>

__FISSION_BEGIN__

namespace base
{

	/**!
	 * @brief Structure representing a 2D size or resolution.
	 */
	template <typename _Ty>
	struct size2
	{
		using type = _Ty;

		type w; // Width
		type h; // Height


		constexpr size2(const size2& src) = default;

		/*! @brief Create null size. */
		constexpr size2():w(static_cast<type>(0)),h(static_cast<type>(0)){}

		/*! @brief Create size from a Width and Height. */
		constexpr size2(type _Width,type _Height):w(_Width),h(_Height){}

		/*! @brief Create size from another size type. */
		template <typename _From>
		constexpr size2(const size2<_From>&_Src):w(static_cast<type>(_Src.w)),h(static_cast<type>(_Src.w)){}

		//! @brief Reinterpret constructor.
		//! @note If the structure is not the same as `_Ty` * 2,
		//!       then the behavior of this function is undefined.
		template <typename _SizeType>
		explicit constexpr size2(const _SizeType&_Src):
			w( *(reinterpret_cast<const type *>( &_Src )) ),
			h( *(reinterpret_cast<const type *>( &_Src )+1) )
		{
			static_assert( sizeof( _SizeType ) == sizeof( size2 ), "Must be same size in order to reinterpret" );
		}

		inline constexpr type&width(){return w;}
		inline constexpr type&height(){return h;}

		inline constexpr type width()const{return w;}
		inline constexpr type height()const{return h;}

		inline constexpr type area()const{return w*h;}

		template <typename _To>
		inline constexpr _To as(){return _To{w,h};}

		constexpr bool operator==(size2 const&) const = default;

	}; // Fission::base::size2
	
} // namespace Fission::base

namespace size_math_operators
{
    template <typename T>inline constexpr auto operator+(const base::size2<T>&_Left,const base::size2<T>&_Right){return size2<T>(_Left.w+_Right.w,_Left.h+_Right.h);}
    template <typename T>inline constexpr auto operator-(const base::size2<T>&_Left,const base::size2<T>&_Right){return size2<T>(_Left.w-_Right.w,_Left.h-_Right.h);}
    template <typename T>inline constexpr auto operator*(const base::size2<T>&_Left,const base::size2<T>&_Right){return size2<T>(_Left.w*_Right.w,_Left.h*_Right.h);}
    template <typename T>inline constexpr auto operator/(const base::size2<T>&_Left,const base::size2<T>&_Right){return size2<T>(_Left.w/_Right.w,_Left.h/_Right.h);}

    template <typename T>inline constexpr auto operator*(const T&_Left,const base::size2<T>&_Right){return size2<T>(_Left*_Right.w,_Left*_Right.h);}
    template <typename T>inline constexpr auto operator*(const base::size2<T>&_Left,const T&_Right){return size2<T>(_Left.w*_Right,_Left.h*_Right);}
    template <typename T>inline constexpr auto operator/(const base::size2<T>&_Left,const T&_Right){return size2<T>(_Left.w/_Right,_Left.h/_Right);}

} // namespace Fission::size_math_operators


__FISSION_BASE_ALIASES__(base::size2, size2);

/* Select Default structure to use for sizes. */
using size2 = size2i32;


__FISSION_END__
