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
constexpr BASE operator- (                 )const{return{-this->X,-this->Y };} \
constexpr BASE operator+ (BASE const& right)const{return{this->X+right.X,this->Y+right.Y};} \
constexpr BASE operator- (BASE const& right)const{return{this->X-right.X,this->Y-right.Y};} \
constexpr BASE operator* (BASE const& right)const{return{this->X*right.X,this->Y*right.Y};} \
constexpr BASE operator/ (BASE const& right)const{return{this->X/right.X,this->Y/right.Y};} \
constexpr BASE&operator+=(BASE const& right)     {this->X+=right.X,this->Y+=right.Y;return*this;} \
constexpr BASE&operator-=(BASE const& right)     {this->X-=right.X,this->Y-=right.Y;return*this;} \
constexpr BASE&operator*=(BASE const& right)     {this->X*=right.X,this->Y*=right.Y;return*this;} \
constexpr BASE&operator/=(BASE const& right)     {this->X/=right.X,this->Y/=right.Y;return*this;} \
constexpr BASE operator* (const TYPE& right)const{return{this->X*right,this->Y*right};} \
constexpr BASE operator/ (const TYPE& right)const{return{this->X/right,this->Y/right};} \
constexpr BASE&operator*=(const TYPE& right)     {this->X*=right,this->Y*=right;return*this;} \
constexpr BASE&operator/=(const TYPE& right)     {this->X/=right,this->Y/=right;return*this;}

#define _FISSION_IMPLEMENT_OPERATOR_MULTIPLY_2(BASE, TYPE, X, Y) \
inline constexpr auto operator*(TYPE const&left,BASE const&right){return BASE{left*right.X,left*right.Y};}

#define _FISSION_IMPLEMENT_OPERATOR_DIVISION_2(BASE, TYPE, X, Y) \
inline constexpr auto operator/(TYPE const&left,BASE const&right){return BASE{left/right.X,left/right.Y};}

#define _FISSION_IMPLEMENT_OPERATOR_DOT_2(BASE, X, Y) \
inline constexpr auto dot(BASE const&left,BASE const&right){return left.X*right.X+left.Y*right.Y;}


#define _FISSION_IMPLEMENT_OPERATORS_3(BASE, TYPE, X, Y, Z) \
constexpr BASE  operator- (                   )const { return { -this->X, -this->Y, -this->Z }; } \
constexpr BASE  operator+ ( BASE const& right )const { return { this->X + right.X, this->Y + right.Y, this->Z + right.Z }; } \
constexpr BASE  operator- ( BASE const& right )const { return { this->X - right.X, this->Y - right.Y, this->Z - right.Z }; } \
constexpr BASE  operator* ( BASE const& right )const { return { this->X * right.X, this->Y * right.Y, this->Z * right.Z }; } \
constexpr BASE  operator/ ( BASE const& right )const { return { this->X / right.X, this->Y / right.Y, this->Z / right.Z }; } \
constexpr BASE& operator+=( BASE const& right )      { this->X += right.X, this->Y += right.Y, this->Z += right.Z; return*this; } \
constexpr BASE& operator-=( BASE const& right )      { this->X -= right.X, this->Y -= right.Y, this->Z -= right.Z; return*this; } \
constexpr BASE& operator*=( BASE const& right )      { this->X *= right.X, this->Y *= right.Y, this->Z *= right.Z; return*this; } \
constexpr BASE& operator/=( BASE const& right )      { this->X /= right.X, this->Y /= right.Y, this->Z /= right.Z; return*this; } \
constexpr BASE  operator* ( const TYPE& right )const { return { this->X * right, this->Y * right, this->Z * right }; } \
constexpr BASE  operator/ ( const TYPE& right )const { return { this->X / right, this->Y / right, this->Z / right }; } \
constexpr BASE& operator*=( const TYPE& right )      { this->X *= right, this->Y *= right, this->Z *= right; return*this; } \
constexpr BASE& operator/=( const TYPE& right )      { this->X /= right, this->Y /= right, this->Z /= right; return*this; }

#define _FISSION_IMPLEMENT_OPERATOR_MULTIPLY_3(BASE, TYPE, X, Y, Z) \
inline constexpr auto operator*(TYPE const&left,BASE const&right){return BASE{left*right.X,left*right.Y,left*right.Z};}

#define _FISSION_IMPLEMENT_OPERATOR_DIVISION_3(BASE, TYPE, X, Y, Z) \
inline constexpr auto operator/(TYPE const&left,BASE const&right){return BASE{left/right.X,left/right.Y,left/right.Z};}

#define _FISSION_IMPLEMENT_OPERATOR_DOT_3(BASE, X, Y, Z) \
inline constexpr auto dot(BASE const&left,BASE const&right){return left.X*right.X+left.Y*right.Y+left.Z*right.Z;}


#define _FISSION_IMPLEMENT_OPERATORS_4(BASE, TYPE, X, Y, Z, W) \
constexpr BASE  operator- (                   )const { return { -this->X, -this->Y, -this->Z, -this->W }; } \
constexpr BASE  operator+ ( BASE const& right )const { return { this->X + right.X, this->Y + right.Y, this->Z + right.Z, this->W + right.W }; } \
constexpr BASE  operator- ( BASE const& right )const { return { this->X - right.X, this->Y - right.Y, this->Z - right.Z, this->W - right.W }; } \
constexpr BASE  operator* ( BASE const& right )const { return { this->X * right.X, this->Y * right.Y, this->Z * right.Z, this->W * right.W }; } \
constexpr BASE  operator/ ( BASE const& right )const { return { this->X / right.X, this->Y / right.Y, this->Z / right.Z, this->W / right.W }; } \
constexpr BASE& operator+=( BASE const& right )      { this->X += right.X, this->Y += right.Y, this->Z += right.Z, this->W += right.W; return*this; } \
constexpr BASE& operator-=( BASE const& right )      { this->X -= right.X, this->Y -= right.Y, this->Z -= right.Z, this->W -= right.W; return*this; } \
constexpr BASE& operator*=( BASE const& right )      { this->X *= right.X, this->Y *= right.Y, this->Z *= right.Z, this->W *= right.W; return*this; } \
constexpr BASE& operator/=( BASE const& right )      { this->X /= right.X, this->Y /= right.Y, this->Z /= right.Z, this->W /= right.W; return*this; } \
constexpr BASE  operator* ( const TYPE& right )const { return { this->X * right, this->Y * right, this->Z * right, this->W * right }; } \
constexpr BASE  operator/ ( const TYPE& right )const { return { this->X / right, this->Y / right, this->Z / right, this->W / right }; } \
constexpr BASE& operator*=( const TYPE& right )      { this->X *= right, this->Y *= right, this->Z *= right, this->W *= right; return*this; } \
constexpr BASE& operator/=( const TYPE& right )      { this->X /= right, this->Y /= right, this->Z /= right, this->W /= right; return*this; }

#define _FISSION_IMPLEMENT_OPERATOR_MULTIPLY_4(BASE, TYPE, X, Y, Z, W) \
inline constexpr BASE operator*(TYPE const&left,BASE const&right){return {left*right.X,left*right.Y,left*right.Z,left*right.W};}

#define _FISSION_IMPLEMENT_OPERATOR_DIVISION_4(BASE, TYPE, X, Y, Z, W) \
inline constexpr BASE operator/(TYPE const&left,BASE const&right){return {left/right.X,left/right.Y,left/right.Z,left/right.W};}

#define _FISSION_IMPLEMENT_OPERATOR_DOT_4(BASE, X, Y, Z, W) \
inline constexpr auto dot(BASE const&left,BASE const&right){return left.X*right.X+left.Y*right.Y+left.Z*right.Z+left.W*right.W;}

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
