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
#include <Fission/base/types.hpp>
#include <Fission/base/math/library.hpp>
#include <Fission/base/impl/operators.hpp>
#include <compare>

FISSION_NAMESPACE_BEGIN

namespace math
{
	template <typename type>
	struct vector2
	{
		type x, y;

		constexpr vector2(vector2 const&) = default;
		constexpr vector2& operator=(vector2 const&) = default;

		/*! @brief Create default vector: {0,0} */
		constexpr vector2():x(static_cast<type>(0)),y(static_cast<type>(0)){}

		/*! @brief Create vector with one value for all components. */
		explicit constexpr vector2(type value):x(value),y(value){}

		/*! @brief Create vector with values X and Y. */
		constexpr vector2(type X,type Y):x(X),y(Y){}

		/*! @brief Create vector from another vector with different component type. */
		template <typename from> explicit
		constexpr vector2(const vector2<from>&v):x(static_cast<type>(v.x)),y(static_cast<type>(v.y)){}

		//! @brief  Construct a vector from a 3rd party vector type.
		//! @param  v : vector object with public variables x and y.
		template <typename vector2_type>
		static constexpr vector2 from(vector2_type const& v){return {v.x,v.y};}


		//! @brief Get vector with flipped components.
		constexpr vector2 yx()const{return {y,x};}

		//! @brief Get the vector that is rotated 90* clockwise from this vector.
		constexpr vector2 perp()const{return {-y,x};}

		//! @brief Get the squared length of this vector.
		//! @note This will always be faster than getting the actual length.
		constexpr type lensq()const{return this->x*this->x+this->y*this->y;}

		//! @brief Get the length of this vector.
		constexpr type len()const{return std_library::sqrt(this->x*this->x+this->y*this->y);}

		//! @brief Get the normal vector for this vector.
		//! @warning UNDEFINED FOR VECTOR {0,0}
		constexpr vector2 norm()const{auto k=len();return vector2(this->x/k,this->y/k);}

		//! @brief Transform vector so that it has a length of one.
		//! @warning UNDEFINED FOR VECTOR {0,0}
		//! @return Reference to this vector.
		constexpr vector2&normalize(){auto k=len();this->x/=k,this->y/=k;return*this;}

		inline constexpr bool operator==(vector2 const&) const = default;

		_FISSION_IMPLEMENT_OPERATORS_2(vector2, type, x, y)

	}; // Fission::base::vector2

	template <typename type>
	struct vector3
	{
		using vec2 = vector2<type>;

		type x, y, z;

		constexpr vector3(vector3 const&) = default;

		/*! @brief Create default vector: {0,0,0} */
		constexpr vector3():x(static_cast<type>(0)),y(static_cast<type>(0)),z(static_cast<type>(0)){}

		/*! @brief Create vector with one value for all components. */
		explicit constexpr vector3(type value):x(value),y(value),z(value){}

		/*! @brief Create vector with values X, Y, and Z. */
		constexpr vector3(type X,type Y,type Z):x(X),y(Y),z(Z){}

		/*! @brief Create vector from an XY vector and Z value. */
		constexpr vector3(vec2 XY,type Z):x(XY.x),y(XY.y),z(Z){}

		/*! @brief Create vector from an X value and YZ vector. */
		constexpr vector3(type X,vec2 YZ):x(X),y(YZ.x),z(YZ.y){}

		/*! @brief Create vector from another vector with different component type. */
		template <typename from> explicit
		constexpr vector3(vector3<from> const&v):x(static_cast<type>(v.x)),y(static_cast<type>(v.y)),z(static_cast<type>(v.z)){}

		//! @brief  Construct a vector from a 3rd party vector type.
		//! @param  v : vector object with public variables x, y, and z.
		template <typename vector3_type>
		static constexpr vector3 from(const vector3_type&v){return {v.x,v.y,v.z};}


		//! @brief Get the squared length of this vector.
		//! @note This will always be faster than getting the actual length.
		constexpr type lensq()const{return this->x*this->x+this->y*this->y+this->z*this->z;}

		//! @brief Get the length of this vector.
		constexpr type len()const{return std_library::sqrt(this->x*this->x+this->y*this->y+this->z*this->z);}

		//! @brief Get the normal vector for this vector.
		//! @warning UNDEFINED FOR VECTOR {0,0,0}
		constexpr vector3 norm()const{auto k=len();return vector3(this->x/k,this->y/k,this->z/k);}

		//! @brief Transform vector so that it has a length of one.
		//! @warning UNDEFINED FOR VECTOR {0,0,0}
		//! @return Reference to this vector.
		constexpr vector3&normalize(){auto k=len();this->x/=k,this->y/=k,this->z/=k;return*this;}


		inline constexpr bool operator==(vector3 const&) const = default;

		_FISSION_IMPLEMENT_OPERATORS_3(vector3, type, x, y, z)

	}; // Fission::base::vector3

	template <typename type>
	struct vector4
	{
		using vec2 = vector2<type>;
		using vec3 = vector3<type>;

		type x, y, z, w;

		constexpr vector4(vector4 const&) = default;

		/*! @brief Create default vector: {0,0,0,0} */
		constexpr vector4():x(static_cast<type>(0)),y(static_cast<type>(0)),z(static_cast<type>(0)),w(static_cast<type>(0)){}

		/*! @brief Create vector with one value for all components. */
		explicit constexpr vector4(type value):x(value),y(value),z(value),w(value){}

		/*! @brief Create vector with values X, Y, Z, and W. */
		constexpr vector4(type X,type Y,type Z,type W):x(X),y(Y),z(Z),w(W){}

		/*! @brief Create vector from an XY vector, Z value, and W value. */
		constexpr vector4(vec2 XY,type Z,type W):x(XY.x),y(XY.y),z(Z),w(W){}
		
		/*! @brief Create vector from an X value, YZ vector, and W value. */
		constexpr vector4(type X,vec2 YZ,type W):x(X),y(YZ.x),z(YZ.y),w(W){}

		/*! @brief Create vector from an X value, Y value, and ZW vector. */
		constexpr vector4(type X,type Y,vec2 ZW):x(X),y(Y),z(ZW.x),w(ZW.y){}

		/*! @brief Create vector from an XY vector and ZW vector. */
		constexpr vector4(vec2 XY,vec2 ZW):x(XY.x),y(XY.y),z(ZW.x),w(ZW.y){}

		/*! @brief Create vector from an XYZ vector and W value. */
		constexpr vector4(vec3 XYZ,type W):x(XYZ.x),y(XYZ.y),z(XYZ.z),w(W){}

		/*! @brief Create vector from an X value and YZW vector. */
		constexpr vector4(type X,vec3 YZW):x(X),y(YZW.x),z(YZW.y),w(YZW.z){}

		/*! @brief Create vector from another vector with different component type. */
		template <typename from> explicit
		constexpr vector4(vector4<from> const&v):x(static_cast<type>(v.x)),y(static_cast<type>(v.y)),z(static_cast<type>(v.z)),w(static_cast<type>(v.w)){}

		//! @brief  Construct a vector from a 3rd party vector type.
		//! @param  v : vector object with public variables x, y, z, and w.
		template <typename vector4_type>
		static constexpr vector4 from(vector4_type const&v){return {v.x,v.y,v.z,v.w};}


		//! @brief Get the squared length of this vector.
		//! @note This will always be faster than getting the actual length.
		constexpr type lensq()const{return this->x*this->x+this->y*this->y+this->z*this->z+this->w*this->w;}

		//! @brief Get the length of this vector.
		constexpr type len()const{return std_library::sqrt(this->x*this->x+this->y*this->y+this->z*this->z+this->w*this->w);}

		//! @brief Get the normal vector for this vector.
		//! @warning UNDEFINED FOR VECTOR {0,0,0,0}
		constexpr vector4 norm()const{auto k=len();return vector4(this->x/k,this->y/k,this->z/k,this->w/k);}

		//! @brief Transform vector so that it has a length of one.
		//! @warning UNDEFINED FOR VECTOR {0,0,0,0}
		//! @return Reference to this vector.
		constexpr vector4&normalize(){auto k=len();this->x/=k,this->y/=k,this->z/=k,this->w/=k;return*this;}


		constexpr bool operator==(vector4 const&) const = default;

		_FISSION_IMPLEMENT_OPERATORS_4(vector4, type, x, y, z, w)

	}; // Fission::base::vector4

}

template<typename type> using v2 = math::vector2<type>;
template<typename type> using v3 = math::vector3<type>;
template<typename type> using v4 = math::vector4<type>;

_FISSION_BASE_ALIASES(math::vector2, v2);
_FISSION_BASE_ALIASES(math::vector3, v3);
_FISSION_BASE_ALIASES(math::vector4, v4);

FISSION_NAMESPACE_END

template <typename T> _FISSION_IMPLEMENT_OPERATOR_MULTIPLY_2(fs::math::vector2<T>, T, x, y)
template <typename T> _FISSION_IMPLEMENT_OPERATOR_MULTIPLY_3(fs::math::vector3<T>, T, x, y, z)
template <typename T> _FISSION_IMPLEMENT_OPERATOR_MULTIPLY_4(fs::math::vector4<T>, T, x, y, z, w)

template <typename T> _FISSION_IMPLEMENT_OPERATOR_DIVISION_2(fs::math::vector2<T>, T, x, y)
template <typename T> _FISSION_IMPLEMENT_OPERATOR_DIVISION_3(fs::math::vector3<T>, T, x, y, z)
template <typename T> _FISSION_IMPLEMENT_OPERATOR_DIVISION_4(fs::math::vector4<T>, T, x, y, z, w)

template <typename T> _FISSION_IMPLEMENT_OPERATOR_DOT_2(fs::math::vector2<T>, x, y)
template <typename T> _FISSION_IMPLEMENT_OPERATOR_DOT_3(fs::math::vector3<T>, x, y, z)
template <typename T> _FISSION_IMPLEMENT_OPERATOR_DOT_4(fs::math::vector4<T>, x, y, z, w)

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
