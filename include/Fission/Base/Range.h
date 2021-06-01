/**
*
* @file: Range.h
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

template <typename _Ty>
struct range
{
    using type = _Ty;

    type low;  // lower bound of the range
    type high; // upper bound of the range

    constexpr range(const range&src) = default;

    //! @brief Create null range: {0,0}.
    constexpr range():low(static_cast<type>(0)),high(static_cast<type>(0)){}

    //! @brief Create range from a low and high value, where _Low < _High.
    constexpr range(const type&_Low,const type&_High):low(_Low),high(_High){}

    //! @brief Converts range from another type.
    template <typename _From> explicit
    constexpr range(const range<_From>&_Src):low(static_cast<type>(_Src.low)),high(static_cast<type>(_Src.high)){}

    //! @brief Creates valid range from two values A and B.
    static inline constexpr range create(const type&_A,const type&_B){return(_A>_B)?range(_B,_A):range(_A,_B);}

    //! @brief Creates valid range from center and span.
    static inline constexpr range from_center(const type&_Center,const type&_Span){type d=_Span/static_cast<type>(2);return range(_Center-d, _Center+d);}


    // Range Functions

    //!! @brief Clamps a value to this range.
    inline constexpr auto clamp(const type&_X)const{return if(_X<this->low)return this->low;if(_X>this->high)return this->high;return _X;}

    //! @brief Checks whether value is contained within open range: ( low, high ).
    inline constexpr bool operator()(const type&_X)const{return(_X>this->low)&&(_X<this->high);}

    //! @brief Checks whether value is contained within closed range: [ low, high ].
    inline constexpr bool operator[](const type&_X)const{return(_X>=this->low)&&(_X<=this->high);}

    //! @brief Checks whether value is contained within ( low, high ].
    inline constexpr bool closed_upper(const type&_X)const{return(_X>this->low)&&(_X<=this->high);}
    
    //! @brief Checks whether value is contained within [ low, high ).
    inline constexpr bool closed_lower(const type&_X)const{return(_X>=this->low)&&(_X<this->high);}

    //! @brief Get the distance between the two end points of this range.
    inline constexpr auto distance()const{return this->high-this->low;}
    inline constexpr auto difference()const{return this->high-this->low;}

    //! @brief Get the value of the center.
    inline constexpr auto center()const{return(this->high+this->low)/static_cast<type>(2);}
    inline constexpr auto average()const{return(this->high+this->low)/static_cast<type>(2);}


    // Modification Functions
    
    //! @brief Get an Expanded range.
    inline constexpr auto expanded(const type&_dx)const{return range(this->low-_dx,this->high+_dx);}

    //! @brief Expands this range
    inline constexpr auto&expand(const type&_dx){this->low-=_dx,this->high+=_dx;return*this;}

    //! @brief Get a Scaled range from center.
    inline constexpr auto scaled(const type&_Scale)const{
        auto _Center=      (this->high+this->low)/static_cast<type>(2),
             _Delta=_Scale*(this->high-this->low)/static_cast<type>(2); 
        return range(_Center-_Delta,_Center+_Delta);
    }

    //! @brief Scales this range from center.
    inline constexpr auto&scale(const type&_Scale){
        auto _Center=      (this->high+this->low)/static_cast<type>(2),
             _Delta=_Scale*(this->high-this->low)/static_cast<type>(2); 
        this->low=_Center-_Delta,this->high=_Center+_Delta;return*this;
    }


    // Operators

    /*! @brief Check if the ranges are equal. */
    inline constexpr auto operator==(const type &_Right) const { return (this->low == _Right.low) && (this->high == _Right.high); }

    /*! @brief Check if the ranges are not equal. */
    inline constexpr auto operator!=(const type &_Right) const { return (this->low != _Right.low) || (this->high != _Right.high); }

    //! @brief Get a range scaled from zero.
    constexpr auto operator*(const type&_Scale)const{return range(this->low*_Scale,this->high*_Scale);}

    //! @brief Scale this range from zero.
    constexpr auto&operator*=(const type&_Scale){this->low*=_Scale,this->high*=_Scale;return*this;}

    // - Shift Operators

    constexpr auto operator+(const type&_Shift)const{return range(this->low+_Shift,this->high+_Shift);}
    constexpr auto operator-(const type&_Shift)const{return range(this->low-_Shift,this->high+_Shift);}

    constexpr auto&operator+=(const type&_Shift){this->low+=_Shift,this->high+=_Shift;return*this;}
    constexpr auto&operator-=(const type&_Shift){this->low-=_Shift,this->high-=_Shift;return*this;}

}; // struct Fission::base::range

//! @return The minimum distance between two ranges.
//! @note: Returns a negative value for ranges that contain a subset of each other.
template <typename T>
inline constexpr auto intersect(const range<T>&_A, const range<T>&_B)
{
    auto _P = _B.low - _A.high, _K = _A.low - _B.high;
    return (_P>_K)?_P:_K;
}

_FISSION_BASE_END

