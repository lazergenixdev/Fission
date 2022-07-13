/**
 * @file Operators.hpp
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
 *	Copyright (c) 2022 Lazergenix
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

#define _FISSION_DEFINE_OPERATORS_2(BASE, TYPE, X, Y) \
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

#define _FISSION_DEFINE_OPERATOR_MULTIPLY_2(BASE, TYPE, X, Y) \
inline constexpr auto operator*(TYPE const&_Left,BASE const&_Right){return BASE{_Left*_Right.X,_Left*_Right.Y};}


#define _FISSION_DEFINE_OPERATORS_3(BASE, TYPE, X, Y, Z) \
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

#define _FISSION_DEFINE_OPERATOR_MULTIPLY_3(BASE, TYPE, X, Y, Z) \
inline constexpr auto operator*(TYPE const&_Left,BASE const&_Right){return BASE{_Left*_Right.X,_Left*_Right.Y,_Left*_Right.Z};}


#define _FISSION_DEFINE_OPERATORS_4(BASE, TYPE, X, Y, Z, W) \
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

#define _FISSION_DEFINE_OPERATOR_MULTIPLY_4(BASE, TYPE, X, Y, Z, W) \
inline constexpr auto operator*(TYPE const&_Left,BASE const&_Right){return BASE{_Left*_Right.X,_Left*_Right.Y,_Left*_Right.Z,_Left*_Right.W};}
