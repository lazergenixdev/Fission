/**
 * @file Vector.hpp
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
#include <Fission/Base/util/Operators.hpp>
#include <compare>

__FISSION_BEGIN__

namespace base
{
	template <typename _Ty>
	struct vector2
	{
		using type = _Ty;

		type x, y;

		constexpr vector2(const vector2&_Src) = default;

		/*! @brief Create default vector: {0,0} */
		constexpr vector2():x(static_cast<type>(0)),y(static_cast<type>(0)){}

		/*! @brief Create vector with values X and Y. */
		constexpr vector2(type _X,type _Y):x(_X),y(_Y){}

		/*! @brief Create vector from another vector with different component type. */
		template <typename _From> explicit
		constexpr vector2(const vector2<_From>&_Src):x(static_cast<type>(_Src.x)),y(static_cast<type>(_Src.y)){}

		//! @brief  Construct a vector from a 3rd party vector type.
		//! @param  _Vector: vector object with public variables x and y.
		template <typename _VecTy>
		static constexpr auto from(const _VecTy&_Vector){return vector2(_Vector.x,_Vector.y);}

		/*! @brief Create vector with one value for all components. */
		static constexpr auto from1(type _Val){return vector2(_Val,_Val);}


		//! @brief Get vector with fliped components.
		constexpr auto yx()const{return vector2(y,x);}

		//! @brief Get the vector that is rotated 90* clockwise from this vector.
		constexpr auto perp()const{return vector2(-y,x);}

		//! @brief Get the squared length of this vector.
		//! @note This will always be faster than getting the actual length.
		constexpr auto lensq()const{return this->x*this->x+this->y*this->y;}

		//! @brief Get the length of this vector.
		constexpr auto len()const{return::sqrt(this->x*this->x+this->y*this->y);}

		//! @brief Get the normal vector for this vector.
		//! @warning UNDEFINED FOR VECTOR {0,0}
		constexpr auto norm()const{auto k=len();return vector2(this->x/k,this->y/k);}

		//! @brief Transform vector so that it has a length of one.
		//! @warning UNDEFINED FOR VECTOR {0,0}
		//! @return Reference to this vector.
		constexpr auto&normalize(){auto k=len();this->x/=k,this->y/=k;return*this;}

		inline constexpr bool operator==(vector2 const&) const = default;
		inline constexpr auto operator<=>(vector2 const&) const = default;

		_FISSION_DEFINE_OPERATORS_2(vector2, type, x, y);

	}; // Fission::base::vector2

	template <typename _Ty>
	struct vector3
	{
		using type = _Ty;
		using vector2 = vector2<_Ty>;

		type x, y, z;

		constexpr vector3(const vector3&_Src) = default;

		/*! @brief Create default vector: {0,0,0} */
		constexpr vector3():x(static_cast<type>(0)),y(static_cast<type>(0)),z(static_cast<type>(0)){}

		/*! @brief Create vector with values X, Y, and Z. */
		constexpr vector3(type _X,type _Y,type _Z):x(_X),y(_Y),z(_Z){}

		/*! @brief Create vector from an XY vector and Z value. */
		constexpr vector3(vector2 _XY,type _Z):x(_XY.x),y(_XY.y),z(_Z){}

		/*! @brief Create vector from an X value and YZ vector. */
		constexpr vector3(type _X,vector2 _YZ):x(_X),y(_YZ.x),z(_YZ.y){}

		/*! @brief Create vector from another vector with different component type. */
		template <typename _From> explicit
		constexpr vector3(const vector3<_From>&_Src):x(static_cast<type>(_Src.x)),y(static_cast<type>(_Src.y)),z(static_cast<type>(_Src.z)){}

		//! @brief  Construct a vector from a 3rd party vector type.
		//! @param  _Vector: vector object with public variables x, y, and z.
		template <typename _VecTy>
		static constexpr auto from(const _VecTy&_Vector){return vector3(_Vector.x,_Vector.y,_Vector.z);}

		/*! @brief Create vector with one value for all components. */
		static constexpr auto from1(type _Val){return vector3(_Val,_Val,_Val);}


		//! @brief Get the squared length of this vector.
		//! @note This will always be faster than getting the actual length.
		constexpr auto lensq()const{return this->x*this->x+this->y*this->y+this->z*this->z;}

		//! @brief Get the length of this vector.
		constexpr auto len()const{return::sqrt(this->x*this->x+this->y*this->y+this->z*this->z);}

		//! @brief Get the normal vector for this vector.
		//! @warning UNDEFINED FOR VECTOR {0,0,0}
		constexpr auto norm()const{auto k=len();return vector3(this->x/k,this->y/k,this->z/k);}

		//! @brief Transform vector so that it has a length of one.
		//! @warning UNDEFINED FOR VECTOR {0,0,0}
		//! @return Reference to this vector.
		constexpr auto&normalize(){auto k=len();this->x/=k,this->y/=k,this->z/=k;return*this;}


		inline constexpr bool operator==(vector3 const&) const = default;
		inline constexpr auto operator<=>(vector3 const&) const = default;

		_FISSION_DEFINE_OPERATORS_3(vector3, type, x, y, z);

	}; // Fission::base::vector3

	template <typename _Ty>
	struct vector4
	{
		using type = _Ty;
		using vector2 = vector2<_Ty>;
		using vector3 = vector3<_Ty>;

		type x, y, z, w;

		constexpr vector4(const vector4&_Src) = default;

		/*! @brief Create default vector: {0,0,0,0} */
		constexpr vector4():x(static_cast<type>(0)),y(static_cast<type>(0)),z(static_cast<type>(0)),w(static_cast<type>(0)){}

		/*! @brief Create vector with values X, Y, Z, and W. */
		constexpr vector4(type _X,type _Y,type _Z,type _W):x(_X),y(_Y),z(_Z),w(_W){}

		/*! @brief Create vector from an XY vector, Z value, and W value. */
		constexpr vector4(vector2 _XY,type _Z,type _W):x(_XY.x),y(_XY.y),z(_Z),w(_W){}
		
		/*! @brief Create vector from an X value, YZ vector, and W value. */
		constexpr vector4(type _X,vector2 _YZ,type _W):x(_X),y(_YZ.x),z(_YZ.y),w(_W){}

		/*! @brief Create vector from an X value, Y value, and ZW vector. */
		constexpr vector4(type _X,type _Y,vector2 _ZW):x(_X),y(_Y),z(_ZW.x),w(_ZW.y){}

		/*! @brief Create vector from an XY vector and ZW vector. */
		constexpr vector4(vector2 _XY,vector2 _ZW):x(_XY.x),y(_XY.y),z(_ZW.x),w(_ZW.y){}

		/*! @brief Create vector from an XYZ vector and W value. */
		constexpr vector4(vector3 _XYZ,type _W):x(_XYZ.x),y(_XYZ.y),z(_XYZ.z),w(_W){}

		/*! @brief Create vector from an X value and YZW vector. */
		constexpr vector4(type _X,vector3 _YZW):x(_X),y(_YZW.x),z(_YZW.y),w(_YZW.z){}

		/*! @brief Create vector from another vector with different component type. */
		template <typename _From> explicit
		constexpr vector4(const vector4<_From>&_Src):x(static_cast<type>(_Src.x)),y(static_cast<type>(_Src.y)),z(static_cast<type>(_Src.z)),w(static_cast<type>(_Src.w)){}

		//! @brief  Construct a vector from a 3rd party vector type.
		//! @param  _Vector: vector object with public variables x, y, z, and w.
		template <typename _VecTy>
		static constexpr auto from(const _VecTy&_Vector){return vector4(_Vector.x,_Vector.y,_Vector.z,_Vector.w);}

		/*! @brief Create vector with one value for all components. */
		static constexpr auto from1(type _Val){return vector4(_Val,_Val,_Val,_Val);}


		//! @brief Get the squared length of this vector.
		//! @note This will always be faster than getting the actual length.
		constexpr auto lensq()const{return this->x*this->x+this->y*this->y+this->z*this->z+this->w*this->w;}

		//! @brief Get the length of this vector.
		constexpr auto len()const{return::sqrt(this->x*this->x+this->y*this->y+this->z*this->z+this->w*this->w);}

		//! @brief Get the normal vector for this vector.
		//! @warning UNDEFINED FOR VECTOR {0,0,0,0}
		constexpr auto norm()const{auto k=len();return vector4(this->x/k,this->y/k,this->z/k,this->w/k);}

		//! @brief Transform vector so that it has a length of one.
		//! @warning UNDEFINED FOR VECTOR {0,0,0,0}
		//! @return Reference to this vector.
		constexpr auto&normalize(){auto k=len();this->x/=k,this->y/=k,this->z/=k,this->w/=k;return*this;}


		constexpr bool operator==(vector4 const&) const = default;
		constexpr auto operator<=>(vector4 const&) const = default;

		_FISSION_DEFINE_OPERATORS_4(vector4, type, x, y, z, w);

	}; // Fission::base::vector4

}

template<typename _Ty> using v2 = base::vector2<_Ty>;
template<typename _Ty> using v3 = base::vector3<_Ty>;
template<typename _Ty> using v4 = base::vector4<_Ty>;

__FISSION_BASE_ALIASES__(base::vector2, v2);
__FISSION_BASE_ALIASES__(base::vector3, v3);
__FISSION_BASE_ALIASES__(base::vector4, v4);

template <typename T> _FISSION_DEFINE_OPERATOR_MULTIPLY_2(base::vector2<T>, T, x, y)
template <typename T> _FISSION_DEFINE_OPERATOR_MULTIPLY_3(base::vector3<T>, T, x, y, z)
template <typename T> _FISSION_DEFINE_OPERATOR_MULTIPLY_4(base::vector4<T>, T, x, y, z, w)

template<typename _Ty>inline constexpr auto operator/(const _Ty&_Left,const v2<_Ty>&_Right){return v2{_Left/_Right.x,_Left/_Right.y};}
template<typename _Ty>inline constexpr auto operator/(const _Ty&_Left,const v3<_Ty>&_Right){return v3{_Left/_Right.x,_Left/_Right.y,_Left/_Right.z};}
template<typename _Ty>inline constexpr auto operator/(const _Ty&_Left,const v4<_Ty>&_Right){return v4{_Left/_Right.x,_Left/_Right.y,_Left/_Right.z,_Left/_Right.w};}

template<typename _Ty>inline constexpr _Ty dot(const v2<_Ty>&_A,const v2<_Ty>&_B){return _A.x*_B.x+_A.y*_B.y;}
template<typename _Ty>inline constexpr _Ty dot(const v3<_Ty>&_A,const v3<_Ty>&_B){return _A.x*_B.x+_A.y*_B.y+_A.z*_B.z;}
template<typename _Ty>inline constexpr _Ty dot(const v4<_Ty>&_A,const v4<_Ty>&_B){return _A.x*_B.x+_A.y*_B.y+_A.z*_B.z+_A.w*_B.w;}

__FISSION_END__
