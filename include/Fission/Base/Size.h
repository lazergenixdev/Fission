/**
*
* @file: Size.h
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
#include "config.h"

_FISSION_BASE_BEGIN

/**!
 * @brief Structure representing a physical size or resolution.
 * 
 * @note: Named 'size2' so that template aliases don't look like words
 *        e.g. 'sizel'.
 * 
 */
template <typename _Ty>
struct size2
{
    using type = _Ty;

    type w; // Width
    type h; // Height

    /*! @brief Create null size. */
    constexpr size2():w(static_cast<type>(0)),h(static_cast<type>(0)){}

    /*! @brief Create size from a Width and Height. */
    constexpr size2(type _Width,type _Height):w(_Width),h(_Height){}

    //! @brief Reinterpret constructor.
    //! @note If the structure is not the same as `_Ty` * 2,
    //!       then the behavior of this function is undefined.
    template <typename _SizeType>
    explicit constexpr size2(_SizeType _Src): w( *(reinterpret_cast<type *>( &_Src )) ), h( *(reinterpret_cast<type *>( &_Src )+1) )
    {
        static_assert( sizeof( _SizeType ) == sizeof( size2 ), "Must be same size in order to reinterpret" );
    }

    constexpr auto&width(){return w;}
    constexpr auto&height(){return h;}

    constexpr auto width()const{return w;}
    constexpr auto height()const{return h;}

    template <typename _To>
    constexpr _To as(){return _To{w,h};}

    constexpr bool operator==(const size2&_Right)const{return(this->w==_Right.w)&&(this->h==_Right.h);}
    constexpr bool operator!=(const size2&_Right)const{return(this->w!=_Right.w)||(this->h!=_Right.h);}

}; // Fission::base::size2


namespace size_math_operators
{
    template <typename T>inline constexpr auto operator+(const size2<T>&_Left,const size2<T>&_Right){return size2<T>(_Left.w+_Right.w,_Left.h+_Right.h);}
    template <typename T>inline constexpr auto operator-(const size2<T>&_Left,const size2<T>&_Right){return size2<T>(_Left.w-_Right.w,_Left.h-_Right.h);}
    template <typename T>inline constexpr auto operator*(const size2<T>&_Left,const size2<T>&_Right){return size2<T>(_Left.w*_Right.w,_Left.h*_Right.h);}
    template <typename T>inline constexpr auto operator/(const size2<T>&_Left,const size2<T>&_Right){return size2<T>(_Left.w/_Right.w,_Left.h/_Right.h);}

    template <typename T>inline constexpr auto operator*(const T&_Left,const size2<T>&_Right){return size2<T>(_Left*_Right.w,_Left*_Right.h);}
    template <typename T>inline constexpr auto operator*(const size2<T>&_Left,const T&_Right){return size2<T>(_Left.w*_Right,_Left.h*_Right);}
    template <typename T>inline constexpr auto operator/(const size2<T>&_Left,const T&_Right){return size2<T>(_Left.w/_Right,_Left.h/_Right);}

} // namespace Fission::base::size_math_operators


_FISSION_BASE_ALIASES(size2);

/* Select Default structure to use for sizes. */
using size = size2<int>;


_FISSION_BASE_END
