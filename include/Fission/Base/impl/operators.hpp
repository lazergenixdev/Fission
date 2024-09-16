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

#define _FISSION_IMPLEMENT_OPERATORS_2(BASE, TYPE, X, Y) \
constexpr BASE  operator- (                    )const { return BASE{ -this->X, -this->Y }; } \
constexpr BASE  operator+ ( BASE const& _Right )const { return BASE{ this->X + _Right.X, this->Y + _Right.Y }; } \
constexpr BASE  operator- ( BASE const& _Right )const { return BASE{ this->X - _Right.X, this->Y - _Right.Y }; } \
constexpr BASE  operator* ( BASE const& _Right )const { return BASE{ this->X * _Right.X, this->Y * _Right.Y }; } \
constexpr BASE  operator/ ( BASE const& _Right )const { return BASE{ this->X / _Right.X, this->Y / _Right.Y }; } \
constexpr BASE& operator+=( BASE const& _Right )      { this->X += _Right.X, this->Y += _Right.Y; return*this; } \
constexpr BASE& operator-=( BASE const& _Right )      { this->X -= _Right.X, this->Y -= _Right.Y; return*this; } \
constexpr BASE& operator*=( BASE const& _Right )      { this->X *= _Right.X, this->Y *= _Right.Y; return*this; } \
constexpr BASE& operator/=( BASE const& _Right )      { this->X /= _Right.X, this->Y /= _Right.Y; return*this; } \
constexpr BASE  operator* ( const TYPE& _Right )const { return BASE{ this->X * _Right, this->Y * _Right }; } \
constexpr BASE  operator/ ( const TYPE& _Right )const { return BASE{ this->X / _Right, this->Y / _Right }; } \
constexpr BASE& operator*=( const TYPE& _Right )      { this->X *= _Right, this->Y *= _Right; return*this; } \
constexpr BASE& operator/=( const TYPE& _Right )      { this->X /= _Right, this->Y /= _Right; return*this; }

#define _FISSION_IMPLEMENT_OPERATOR_MULTIPLY_2(BASE, TYPE, X, Y) \
inline constexpr auto operator*(TYPE const&_Left,BASE const&_Right){return BASE{_Left*_Right.X,_Left*_Right.Y};}

#define _FISSION_IMPLEMENT_OPERATOR_DIVISION_2(BASE, TYPE, X, Y) \
inline constexpr auto operator/(TYPE const&_Left,BASE const&_Right){return BASE{_Left/_Right.X,_Left/_Right.Y};}

#define _FISSION_IMPLEMENT_OPERATOR_DOT_2(BASE, X, Y) \
inline constexpr auto dot(BASE const&_Left,BASE const&_Right){return _Left.X*_Right.X+_Left.Y*_Right.Y;}


#define _FISSION_IMPLEMENT_OPERATORS_3(BASE, TYPE, X, Y, Z) \
constexpr BASE  operator- (                    )const { return BASE{ -this->X, -this->Y, -this->Z }; } \
constexpr BASE  operator+ ( BASE const& _Right )const { return BASE{ this->X + _Right.X, this->Y + _Right.Y, this->Z + _Right.Z }; } \
constexpr BASE  operator- ( BASE const& _Right )const { return BASE{ this->X - _Right.X, this->Y - _Right.Y, this->Z - _Right.Z }; } \
constexpr BASE  operator* ( BASE const& _Right )const { return BASE{ this->X * _Right.X, this->Y * _Right.Y, this->Z * _Right.Z }; } \
constexpr BASE  operator/ ( BASE const& _Right )const { return BASE{ this->X / _Right.X, this->Y / _Right.Y, this->Z / _Right.Z }; } \
constexpr BASE& operator+=( BASE const& _Right )      { this->X += _Right.X, this->Y += _Right.Y, this->Z += _Right.Z; return*this; } \
constexpr BASE& operator-=( BASE const& _Right )      { this->X -= _Right.X, this->Y -= _Right.Y, this->Z -= _Right.Z; return*this; } \
constexpr BASE& operator*=( BASE const& _Right )      { this->X *= _Right.X, this->Y *= _Right.Y, this->Z *= _Right.Z; return*this; } \
constexpr BASE& operator/=( BASE const& _Right )      { this->X /= _Right.X, this->Y /= _Right.Y, this->Z /= _Right.Z; return*this; } \
constexpr BASE  operator* ( const TYPE& _Right )const { return BASE{ this->X * _Right, this->Y * _Right, this->Z * _Right }; } \
constexpr BASE  operator/ ( const TYPE& _Right )const { return BASE{ this->X / _Right, this->Y / _Right, this->Z / _Right }; } \
constexpr BASE& operator*=( const TYPE& _Right )      { this->X *= _Right, this->Y *= _Right, this->Z *= _Right; return*this; } \
constexpr BASE& operator/=( const TYPE& _Right )      { this->X /= _Right, this->Y /= _Right, this->Z /= _Right; return*this; }

#define _FISSION_IMPLEMENT_OPERATOR_MULTIPLY_3(BASE, TYPE, X, Y, Z) \
inline constexpr auto operator*(TYPE const&_Left,BASE const&_Right){return BASE{_Left*_Right.X,_Left*_Right.Y,_Left*_Right.Z};}

#define _FISSION_IMPLEMENT_OPERATOR_DIVISION_3(BASE, TYPE, X, Y, Z) \
inline constexpr auto operator/(TYPE const&_Left,BASE const&_Right){return BASE{_Left/_Right.X,_Left/_Right.Y,_Left/_Right.Z};}

#define _FISSION_IMPLEMENT_OPERATOR_DOT_3(BASE, X, Y, Z) \
inline constexpr auto dot(BASE const&_Left,BASE const&_Right){return _Left.X*_Right.X+_Left.Y*_Right.Y+_Left.Z*_Right.Z;}


#define _FISSION_IMPLEMENT_OPERATORS_4(BASE, TYPE, X, Y, Z, W) \
constexpr BASE  operator- (                    )const { return BASE{ -this->X, -this->Y, -this->Z, -this->W }; } \
constexpr BASE  operator+ ( BASE const& _Right )const { return BASE{ this->X + _Right.X, this->Y + _Right.Y, this->Z + _Right.Z, this->W + _Right.W }; } \
constexpr BASE  operator- ( BASE const& _Right )const { return BASE{ this->X - _Right.X, this->Y - _Right.Y, this->Z - _Right.Z, this->W - _Right.W }; } \
constexpr BASE  operator* ( BASE const& _Right )const { return BASE{ this->X * _Right.X, this->Y * _Right.Y, this->Z * _Right.Z, this->W * _Right.W }; } \
constexpr BASE  operator/ ( BASE const& _Right )const { return BASE{ this->X / _Right.X, this->Y / _Right.Y, this->Z / _Right.Z, this->W / _Right.W }; } \
constexpr BASE& operator+=( BASE const& _Right )      { this->X += _Right.X, this->Y += _Right.Y, this->Z += _Right.Z, this->W += _Right.W; return*this; } \
constexpr BASE& operator-=( BASE const& _Right )      { this->X -= _Right.X, this->Y -= _Right.Y, this->Z -= _Right.Z, this->W -= _Right.W; return*this; } \
constexpr BASE& operator*=( BASE const& _Right )      { this->X *= _Right.X, this->Y *= _Right.Y, this->Z *= _Right.Z, this->W *= _Right.W; return*this; } \
constexpr BASE& operator/=( BASE const& _Right )      { this->X /= _Right.X, this->Y /= _Right.Y, this->Z /= _Right.Z, this->W /= _Right.W; return*this; } \
constexpr BASE  operator* ( const TYPE& _Right )const { return BASE{ this->X * _Right, this->Y * _Right, this->Z * _Right, this->W * _Right }; } \
constexpr BASE  operator/ ( const TYPE& _Right )const { return BASE{ this->X / _Right, this->Y / _Right, this->Z / _Right, this->W / _Right }; } \
constexpr BASE& operator*=( const TYPE& _Right )      { this->X *= _Right, this->Y *= _Right, this->Z *= _Right, this->W *= _Right; return*this; } \
constexpr BASE& operator/=( const TYPE& _Right )      { this->X /= _Right, this->Y /= _Right, this->Z /= _Right, this->W /= _Right; return*this; }

#define _FISSION_IMPLEMENT_OPERATOR_MULTIPLY_4(BASE, TYPE, X, Y, Z, W) \
inline constexpr auto operator*(TYPE const&_Left,BASE const&_Right){return BASE{_Left*_Right.X,_Left*_Right.Y,_Left*_Right.Z,_Left*_Right.W};}

#define _FISSION_IMPLEMENT_OPERATOR_DIVISION_4(BASE, TYPE, X, Y, Z, W) \
inline constexpr auto operator/(TYPE const&_Left,BASE const&_Right){return BASE{_Left/_Right.X,_Left/_Right.Y,_Left/_Right.Z,_Left/_Right.W};}

#define _FISSION_IMPLEMENT_OPERATOR_DOT_4(BASE, X, Y, Z, W) \
inline constexpr auto dot(BASE const&_Left,BASE const&_Right){return _Left.X*_Right.X+_Left.Y*_Right.Y+_Left.Z*_Right.Z+_Left.W*_Right.W;}

/**
 *	MIT License
 *
 *	Copyright (c) 2022-2025 lazergenixdev
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
