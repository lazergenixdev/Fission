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
#include <Fission/Base/Math/Vector.hpp>

__FISSION_BEGIN__

/**!
 *
 * @note:
 *   All matricies are column major, this should theoretically be faster for 
 *   performing matrix calculations, but there is probably no actual difference.
 * 
 *  *faster because for simd calculations, there only needs to be alignment 
 *     for the columns to fit into xmm registers, but this is up to the compiler
 *     on how it optimizes the code.
 *
 * 
 *   INDEX:
 * 
 * [Matrix Types]
 * [Helper Types]
 * [Matrix Definitions]
 * [Matrix Aliases]
 * 
*/

namespace math
{

/* ========================================= [Matrix Types] ========================================= */

	template<typename> struct matrix2x2;
	template<typename> struct matrix2x3;
	template<typename> struct matrix3x3;
	template<typename> struct matrix3x4;
	template<typename> struct matrix4x4;

/* ========================================= [Helper Types] ========================================= */

	template <typename _VectorType> struct xy
	{
		using vector = _VectorType;
		vector x, y;
	};
	template <typename _VectorType> struct xyz
	{
		using vector = _VectorType;
		vector x, y, z;
	};
	template <typename _VectorType> struct xyzw
	{
		using vector = _VectorType;
		vector x, y, z, w;
	};


/* ====================================== [Matrix Definitions] ====================================== */

	template <typename _ElemTy>
	struct matrix2x2
	{
		using type = _ElemTy;
		using vector = vector2<_ElemTy>;

	/**!
	 *     Matrix Structure:
	 * 
	 *     | m11 m12 |
	 *     | m21 m22 |
	*/
		type m11, m21,
			m12, m22;


		constexpr matrix2x2(const matrix2x2&_Src) = default;

		//! @brief Create a null matrix.
		constexpr matrix2x2():
			m11(static_cast<type>(0)),m12(static_cast<type>(0)),
			m21(static_cast<type>(0)),m22(static_cast<type>(0))
		{}

		//! @brief Create Matrix from each element.
		constexpr matrix2x2(
			type _Elem_11,type _Elem_12,
			type _Elem_21,type _Elem_22
		):
			m11(_Elem_11),m12(_Elem_12),
			m21(_Elem_21),m22(_Elem_22)
		{}

		//! @brief Create Matrix from two vectors.
		constexpr matrix2x2(vector _I_Vector,vector _J_Vector):
			m11(_I_Vector.x),m12(_J_Vector.x),
			m21(_I_Vector.y),m22(_J_Vector.y)
		{}

		//! @brief Create matrix from another matrix with a different element type.
		template <typename _To>
		constexpr matrix2x2(const matrix2x2<_To>&_Src):
			m11(static_cast<type>(_Src.m11)),m12(static_cast<type>(_Src.m12)),
			m21(static_cast<type>(_Src.m21)),m22(static_cast<type>(_Src.m22))
		{}


		//! @brief Get the Identity matrix.
		static inline constexpr auto Identity(){
			return matrix2x2(
				static_cast<type>(1),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(1)
			);
		}

		//! @brief Create a scaling matrix.
		static inline constexpr auto Scaling(const type&_ScaleX,const type&_ScaleY){
			return matrix2x2(
				_ScaleX,static_cast<type>(0),
				static_cast<type>(0),_ScaleY
			);
		}

		//! @brief Create a rotation matrix.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto Rotation(const type&_Theta){
			auto _cos = _Math_Library::cos(_Theta), _sin = _Math_Library::sin(_Theta);
			return matrix2x2(
				_cos,-_sin,
				_sin, _cos 
			);
		}
		
		//! @brief Get the transposed form of this matrix
		inline constexpr auto transpose(){return matrix2x2(
			m11, m21,
			m12, m22
		);}

		constexpr const auto&operator->()const{return reinterpret_cast<const xy<vector>*>(this);}
		constexpr auto&operator->(){return reinterpret_cast<xy<vector>*>(this);}

		constexpr auto getX()const{return vector(m11,m21);}
		constexpr auto getY()const{return vector(m12,m22);}

		constexpr auto operator*(const vector&_Right)const{return vector(
			m11*_Right.x + m12*_Right.y,
			m21*_Right.x + m22*_Right.y
		);}
		constexpr auto operator()(const vector&_Right)const{return vector(
			m11*_Right.x + m12*_Right.y,
			m21*_Right.x + m22*_Right.y
		);}

		constexpr auto operator*(const matrix2x2&_Right)const{return matrix2x2(
			*this*_Right.getX(), *this*_Right.getY()
		);}
		constexpr auto operator()(const matrix2x2&_Right)const{return matrix2x2(
			*this*_Right.getX(), *this*_Right.getY()
		);}

	}; // Fission::base::matrix2x2

	template <typename _ElemTy>
	struct matrix2x3
	{
		using type = _ElemTy;
		using vector = vector2<_ElemTy>;

	/**!
	 *     Matrix Structure:
	 * 
	 *     | m11 m12 m13 |
	 *     | m21 m22 m23 |
	*/
		type m11, m21,
			m12, m22,
			m13, m23;


		constexpr matrix2x3(const matrix2x3&_Src) = default;

		//! @brief Create a null matrix.
		constexpr matrix2x3():
			m11(static_cast<type>(0)),m12(static_cast<type>(0)),m13(static_cast<type>(0)),
			m21(static_cast<type>(0)),m22(static_cast<type>(0)),m23(static_cast<type>(0))
		{}

		//! @brief Create Matrix from each element.
		constexpr matrix2x3(
			type _Elem_11,type _Elem_12,type _Elem_13,
			type _Elem_21,type _Elem_22,type _Elem_23
		):
			m11(_Elem_11),m12(_Elem_12),m13(_Elem_13),
			m21(_Elem_21),m22(_Elem_22),m23(_Elem_23)
		{}

		//! @brief Create Matrix from three vectors.
		constexpr matrix2x3(vector _I_Vector,vector _J_Vector,vector _K_Vector):
			m11(_I_Vector.x),m12(_J_Vector.x),m13(_K_Vector.x),
			m21(_I_Vector.y),m22(_J_Vector.y),m23(_K_Vector.y)
		{}

		//! @brief Create matrix from another matrix with a different element type.
		template <typename _To>
		constexpr matrix2x3(const matrix2x3<_To>&_Src):
			m11(static_cast<type>(_Src.m11)),m12(static_cast<type>(_Src.m12)),m13(static_cast<type>(_Src.m13)),
			m21(static_cast<type>(_Src.m21)),m22(static_cast<type>(_Src.m22)),m23(static_cast<type>(_Src.m23))
		{}


		//! @brief Get the Identity matrix.
		static inline constexpr auto Identity(){
			return matrix2x3(
				static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(1),static_cast<type>(0)
			);
		}

		//! @brief Create a scaling matrix.
		static inline constexpr auto Scaling(const type&_ScaleX,const type&_ScaleY){
			return matrix2x3(
				_ScaleX             ,static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),_ScaleY             ,static_cast<type>(0)
			);
		}
		
		//! @brief Create a translation matrix.
		static inline constexpr auto Translation(const type&_X,const type&_Y){
			return matrix2x3(
				static_cast<type>(1),static_cast<type>(0),_X,
				static_cast<type>(0),static_cast<type>(1),_Y
			);
		}

		//! @brief Create a rotation matrix.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto Rotation(const type&_Theta){
			auto _cos = _Math_Library::cos(_Theta), _sin = _Math_Library::sin(_Theta);
			return matrix2x3(
				_cos,-_sin,static_cast<type>(0),
				_sin,_cos ,static_cast<type>(0)
			);
		}

		constexpr const auto&operator->()const{return reinterpret_cast<const xyz<vector>*>(this);}
		constexpr auto&operator->(){return reinterpret_cast<xyz<vector>*>(this);}

		constexpr auto getX()const{return vector(m11,m21);}
		constexpr auto getY()const{return vector(m12,m22);}
		constexpr auto getZ()const{return vector(m13,m23);}

		constexpr auto operator*(const vector&_Right)const{return vector(
			m11*_Right.x + m12*_Right.y + m13,
			m21*_Right.x + m22*_Right.y + m23
		);}
		constexpr auto operator()(const vector&_Right)const{return vector(
			m11*_Right.x + m12*_Right.y + m13,
			m21*_Right.x + m22*_Right.y + m23
		);}

		constexpr auto operator*(const matrix2x3&_Right)const{return matrix2x3(
			*this*_Right.getX(), *this*_Right.getY(), *this*_Right.getZ()
		);}
		constexpr auto operator()(const matrix2x3&_Right)const{return matrix2x3(
			*this*_Right.getX(), *this*_Right.getY(), *this*_Right.getZ()
		);}

	}; // Fission::base::matrix2x3

	template <typename _ElemTy>
	struct matrix3x3
	{
		using type = _ElemTy;
		using vector = vector3<_ElemTy>;

	/**!
	 *     Matrix Structure:
	 * 
	 *     | m11 m12 m13 |
	 *     | m21 m22 m23 |
	 *     | m31 m32 m33 |
	*/
		type m11, m21, m31,
			m12, m22, m32,
			m13, m23, m33;


		constexpr matrix3x3(const matrix3x3&_Src) = default;

		//! @brief Create a null matrix.
		constexpr matrix3x3():
			m11(static_cast<type>(0)),m12(static_cast<type>(0)),m13(static_cast<type>(0)),
			m21(static_cast<type>(0)),m22(static_cast<type>(0)),m23(static_cast<type>(0)),
			m31(static_cast<type>(0)),m32(static_cast<type>(0)),m33(static_cast<type>(0))
		{}

		//! @brief Create Matrix from each element.
		constexpr matrix3x3(
			type _Elem_11,type _Elem_12,type _Elem_13,
			type _Elem_21,type _Elem_22,type _Elem_23,
			type _Elem_31,type _Elem_32,type _Elem_33
		):
			m11(_Elem_11),m12(_Elem_12),m13(_Elem_13),
			m21(_Elem_21),m22(_Elem_22),m23(_Elem_23),
			m31(_Elem_31),m32(_Elem_32),m33(_Elem_33)
		{}

		//! @brief Create Matrix from three vectors.
		constexpr matrix3x3(vector _I_Vector,vector _J_Vector,vector _K_Vector):
			m11(_I_Vector.x),m12(_J_Vector.x),m13(_K_Vector.x),
			m21(_I_Vector.y),m22(_J_Vector.y),m23(_K_Vector.y),
			m31(_I_Vector.z),m32(_J_Vector.z),m33(_K_Vector.z)
		{}

		//! @brief Create matrix from another matrix with a different element type.
		template <typename _To>
		constexpr matrix3x3(const matrix3x3<_To>&_Src):
			m11(static_cast<type>(_Src.m11)),m12(static_cast<type>(_Src.m12)),m13(static_cast<type>(_Src.m13)),
			m21(static_cast<type>(_Src.m21)),m22(static_cast<type>(_Src.m22)),m23(static_cast<type>(_Src.m23)),
			m31(static_cast<type>(_Src.m31)),m32(static_cast<type>(_Src.m32)),m33(static_cast<type>(_Src.m33))
		{}


		//! @brief Get the Identity matrix.
		static inline constexpr auto Identity(){
			return matrix3x3(
				static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(1),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(1)
			);
		}

		//! @brief Create a scaling matrix.
		static inline constexpr auto Scaling(const type&_ScaleX,const type&_ScaleY,const type&_ScaleZ){
			return matrix3x3(
				_ScaleX             ,static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),_ScaleY             ,static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),_ScaleZ
			);
		}

		//! @brief Create a rotation matrix around the X axis.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto RotationX(const type&_Theta){
			auto _cos = _Math_Library::cos(_Theta), _sin = _Math_Library::sin(_Theta);
			return matrix3x3(
				_cos                ,-_sin               ,static_cast<type>(0),
				_sin                ,_cos                ,static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(1)
			);
		}

		//! @brief Create a rotation matrix around the Y axis.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto RotationY(const type&_Theta){
			auto _cos = _Math_Library::cos(_Theta), _sin = _Math_Library::sin(_Theta);
			return matrix3x3(
				_cos                ,static_cast<type>(0),_sin                ,
				static_cast<type>(0),static_cast<type>(1),static_cast<type>(0),
				-_sin               ,static_cast<type>(0),_cos
			);
		}

		//! @brief Create a rotation matrix around the Z axis.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto RotationZ(const type&_Theta){
			auto _cos = _Math_Library::cos(_Theta), _sin = _Math_Library::sin(_Theta);
			return matrix3x3(
				static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),_cos                ,-_sin               ,
				static_cast<type>(0),_sin                ,_cos
			);
		}

		//! @brief Create a rotation matrix.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto Rotation(const type&_Yaw,const type&_Pitch,const type&_Roll){
			auto ax = _Math_Library::cos(_Yaw),   ay = _Math_Library::sin(_Yaw);
			auto bx = _Math_Library::cos(_Pitch), by = _Math_Library::sin(_Pitch);
			auto cx = _Math_Library::cos(_Roll),  cy = _Math_Library::sin(_Roll);
			return matrix3x3(
				ax * bx, ax * by*cy - ay*cx , ax * by*cx + ay*cy,
				ay * bx, ay * by*cy + ax*cx , ay * by*cx - ax*cy,
				-by     , bx * cy            , bx * cx
			);
		}
		
		//! @brief Get the transposed form of this matrix
		inline constexpr auto transpose(){return matrix3x3(
			m11, m21, m31,
			m12, m22, m32,
			m13, m23, m33
		);}

		constexpr const auto&operator->()const{return reinterpret_cast<const xyz<vector>*>(this);}
		constexpr auto&operator->(){return reinterpret_cast<xyz<vector>*>(this);}

		constexpr auto getX()const{return vector(m11,m21,m31);}
		constexpr auto getY()const{return vector(m12,m22,m32);}
		constexpr auto getZ()const{return vector(m13,m23,m33);}

		constexpr auto operator*(const vector&_Right)const{return vector(
			m11*_Right.x + m12*_Right.y + m13*_Right.z,
			m21*_Right.x + m22*_Right.y + m23*_Right.z,
			m31*_Right.x + m32*_Right.y + m33*_Right.z
		);}
		constexpr auto operator()(const vector&_Right)const{return vector(
			m11*_Right.x + m12*_Right.y + m13*_Right.z,
			m21*_Right.x + m22*_Right.y + m23*_Right.z,
			m31*_Right.x + m32*_Right.y + m33*_Right.z
		);}

		constexpr auto operator*(const matrix3x3&_Right)const{return matrix3x3(
			*this*_Right.getX(), *this*_Right.getY(), *this*_Right.getZ()
		);}
		constexpr auto operator()(const matrix3x3&_Right)const{return matrix3x3(
			*this*_Right.getX(), *this*_Right.getY(), *this*_Right.getZ()
		);}

	}; // Fission::base::matrix3x3

	template <typename _ElemTy>
	struct matrix3x4
	{
		using type = _ElemTy;
		using vector = vector3<_ElemTy>;

	/**!
	 *     Matrix Structure:
	 * 
	 *     | m11 m12 m13 m14 |
	 *     | m21 m22 m23 m24 |
	 *     | m31 m32 m33 m34 |
	*/
		type m11, m21, m31,
			m12, m22, m32,
			m13, m23, m33,
			m14, m24, m34;


		constexpr matrix3x4(const matrix3x4&_Src) = default;

		//! @brief Create a null matrix.
		constexpr matrix3x4():
			m11(static_cast<type>(0)),m12(static_cast<type>(0)),m13(static_cast<type>(0)),m14(static_cast<type>(0)),
			m21(static_cast<type>(0)),m22(static_cast<type>(0)),m23(static_cast<type>(0)),m24(static_cast<type>(0)),
			m31(static_cast<type>(0)),m32(static_cast<type>(0)),m33(static_cast<type>(0)),m34(static_cast<type>(0))
		{}

		//! @brief Create Matrix from each element.
		constexpr matrix3x4(
			type _Elem_11,type _Elem_12,type _Elem_13,type _Elem_14,
			type _Elem_21,type _Elem_22,type _Elem_23,type _Elem_24,
			type _Elem_31,type _Elem_32,type _Elem_33,type _Elem_34
		):
			m11(_Elem_11),m12(_Elem_12),m13(_Elem_13),m14(_Elem_14),
			m21(_Elem_21),m22(_Elem_22),m23(_Elem_23),m24(_Elem_24),
			m31(_Elem_31),m32(_Elem_32),m33(_Elem_33),m34(_Elem_34)
		{}

		//! @brief Create Matrix from three vectors.
		constexpr matrix3x4(vector _I_Vector,vector _J_Vector,vector _K_Vector,vector _W_Vector):
			m11(_I_Vector.x),m12(_J_Vector.x),m13(_K_Vector.x),m14(_W_Vector.x),
			m21(_I_Vector.y),m22(_J_Vector.y),m23(_K_Vector.y),m24(_W_Vector.y),
			m31(_I_Vector.z),m32(_J_Vector.z),m33(_K_Vector.z),m34(_W_Vector.z)
		{}

		//! @brief Create matrix from another matrix with a different element type.
		template <typename _To>
		constexpr matrix3x4(const matrix3x4<_To>&_Src):
			m11(static_cast<type>(_Src.m11)),m12(static_cast<type>(_Src.m12)),m13(static_cast<type>(_Src.m13)),m14(static_cast<type>(_Src.m14)),
			m21(static_cast<type>(_Src.m21)),m22(static_cast<type>(_Src.m22)),m23(static_cast<type>(_Src.m23)),m24(static_cast<type>(_Src.m24)),
			m31(static_cast<type>(_Src.m31)),m32(static_cast<type>(_Src.m32)),m33(static_cast<type>(_Src.m33)),m34(static_cast<type>(_Src.m34))
		{}


		//! @brief Get the Identity matrix.
		static inline constexpr auto Identity(){
			return matrix3x4(
				static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(1),static_cast<type>(0)
			);
		}

		//! @brief Create a scaling matrix.
		static inline constexpr auto Scaling(const type&_ScaleX,const type&_ScaleY,const type&_ScaleZ){
			return matrix3x4(
				_ScaleX             ,static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),_ScaleY             ,static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),_ScaleZ             ,static_cast<type>(0)
			);
		}
		
		//! @brief Create a translation matrix.
		static inline constexpr auto Translation(const type&_X,const type&_Y,const type&_Z){
			return matrix3x4(
				static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),_X,
				static_cast<type>(0),static_cast<type>(1),static_cast<type>(0),_Y,
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(1),_Z
			);
		}

		//! @brief Create a rotation matrix around the X axis.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto RotationX(const type&_Theta){
			auto _cos = _Math_Library::cos(_Theta), _sin = _Math_Library::sin(_Theta);
			return matrix3x4(
				_cos                ,-_sin               ,static_cast<type>(0),static_cast<type>(0),
				_sin                ,_cos                ,static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(1),static_cast<type>(0)
			);
		}

		//! @brief Create a rotation matrix around the Y axis.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto RotationY(const type&_Theta){
			auto _cos = _Math_Library::cos(_Theta), _sin = _Math_Library::sin(_Theta);
			return matrix3x4(
				_cos                ,static_cast<type>(0),_sin                ,static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),
				-_sin               ,static_cast<type>(0),_cos                ,static_cast<type>(0)
			);
		}

		//! @brief Create a rotation matrix around the Z axis.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto RotationZ(const type&_Theta){
			auto _cos = _Math_Library::cos(_Theta), _sin = _Math_Library::sin(_Theta);
			return matrix3x4(
				static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),_cos                ,-_sin               ,static_cast<type>(0),
				static_cast<type>(0),_sin                ,_cos                ,static_cast<type>(0)
			);
		}

		//! @brief Create a rotation matrix.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto Rotation(const type&_Yaw,const type&_Pitch,const type&_Roll){
			auto ax = _Math_Library::cos(_Yaw),   ay = _Math_Library::sin(_Yaw);
			auto bx = _Math_Library::cos(_Pitch), by = _Math_Library::sin(_Pitch);
			auto cx = _Math_Library::cos(_Roll),  cy = _Math_Library::sin(_Roll);
			return matrix3x4(
				ax * bx, ax * by*cy - ay*cx , ax * by*cx + ay*cy ,static_cast<type>(0),
				ay * bx, ay * by*cy + ax*cx , ay * by*cx - ax*cy ,static_cast<type>(0),
				-by     , bx * cy            , bx * cx            ,static_cast<type>(0)
			);
		}

		constexpr const auto&operator->()const{return reinterpret_cast<const xyzw<vector>*>(this);}
		constexpr auto&operator->(){return reinterpret_cast<xyzw<vector>*>(this);}

		constexpr auto getX()const{return vector(m11,m21,m31);}
		constexpr auto getY()const{return vector(m12,m22,m32);}
		constexpr auto getZ()const{return vector(m13,m23,m33);}
		constexpr auto getW()const{return vector(m14,m24,m34);}

		constexpr auto operator*(const vector&_Right)const{return vector(
			m11*_Right.x + m12*_Right.y + m13*_Right.z + m14,
			m21*_Right.x + m22*_Right.y + m23*_Right.z + m24,
			m31*_Right.x + m32*_Right.y + m33*_Right.z + m34
		);}
		constexpr auto operator()(const vector&_Right)const{return vector(
			m11*_Right.x + m12*_Right.y + m13*_Right.z + m14,
			m21*_Right.x + m22*_Right.y + m23*_Right.z + m24,
			m31*_Right.x + m32*_Right.y + m33*_Right.z + m34
		);}

		constexpr auto operator*(const matrix3x4&_Right)const{return matrix3x4(
			*this*_Right.getX(), *this*_Right.getY(), *this*_Right.getZ(), *this*_Right.getW()
		);}
		constexpr auto operator()(const matrix3x4&_Right)const{return matrix3x4(
			*this*_Right.getX(), *this*_Right.getY(), *this*_Right.getZ(), *this*_Right.getW()
		);}

	}; // Fission::base::matrix3x4

	template <typename _ElemTy>
	struct matrix4x4
	{
		using type = _ElemTy;
		using vector = vector4<_ElemTy>;

	/**!
	 *     Matrix Structure:
	 * 
	 *     | m11 m12 m13 m14 |
	 *     | m21 m22 m23 m24 |
	 *     | m31 m32 m33 m34 |
	 *     | m41 m42 m43 m44 |
	*/
		type m11, m21, m31, m41,
			m12, m22, m32, m42,
			m13, m23, m33, m43,
			m14, m24, m34, m44;


		constexpr matrix4x4(const matrix4x4&_Src) = default;

		//! @brief Create a null matrix.
		constexpr matrix4x4():
			m11(static_cast<type>(0)),m12(static_cast<type>(0)),m13(static_cast<type>(0)),m14(static_cast<type>(0)),
			m21(static_cast<type>(0)),m22(static_cast<type>(0)),m23(static_cast<type>(0)),m24(static_cast<type>(0)),
			m31(static_cast<type>(0)),m32(static_cast<type>(0)),m33(static_cast<type>(0)),m34(static_cast<type>(0)),
			m41(static_cast<type>(0)),m42(static_cast<type>(0)),m43(static_cast<type>(0)),m44(static_cast<type>(0))
		{}

		//! @brief Create Matrix from each element.
		constexpr matrix4x4(
			type _Elem_11,type _Elem_12,type _Elem_13,type _Elem_14,
			type _Elem_21,type _Elem_22,type _Elem_23,type _Elem_24,
			type _Elem_31,type _Elem_32,type _Elem_33,type _Elem_34,
			type _Elem_41,type _Elem_42,type _Elem_43,type _Elem_44
		):
			m11(_Elem_11),m12(_Elem_12),m13(_Elem_13),m14(_Elem_14),
			m21(_Elem_21),m22(_Elem_22),m23(_Elem_23),m24(_Elem_24),
			m31(_Elem_31),m32(_Elem_32),m33(_Elem_33),m34(_Elem_34),
			m41(_Elem_41),m42(_Elem_42),m43(_Elem_43),m44(_Elem_44)
		{}

		//! @brief Create Matrix from three vectors.
		constexpr matrix4x4(vector _I_Vector,vector _J_Vector,vector _K_Vector,vector _W_Vector):
			m11(_I_Vector.x),m12(_J_Vector.x),m13(_K_Vector.x),m14(_W_Vector.x),
			m21(_I_Vector.y),m22(_J_Vector.y),m23(_K_Vector.y),m24(_W_Vector.y),
			m31(_I_Vector.z),m32(_J_Vector.z),m33(_K_Vector.z),m34(_W_Vector.z),
			m41(_I_Vector.w),m42(_J_Vector.w),m43(_K_Vector.w),m44(_W_Vector.w)
		{}

		//! @brief Create matrix from another matrix with a different element type.
		template <typename _To>
		constexpr matrix4x4(const matrix4x4<_To>&_Src):
			m11(static_cast<type>(_Src.m11)),m12(static_cast<type>(_Src.m12)),m13(static_cast<type>(_Src.m13)),m14(static_cast<type>(_Src.m14)),
			m21(static_cast<type>(_Src.m21)),m22(static_cast<type>(_Src.m22)),m23(static_cast<type>(_Src.m23)),m24(static_cast<type>(_Src.m24)),
			m31(static_cast<type>(_Src.m31)),m32(static_cast<type>(_Src.m32)),m33(static_cast<type>(_Src.m33)),m34(static_cast<type>(_Src.m34)),
			m41(static_cast<type>(_Src.m41)),m42(static_cast<type>(_Src.m42)),m43(static_cast<type>(_Src.m43)),m44(static_cast<type>(_Src.m44))
		{}


		//! @brief Get the Identity matrix.
		static inline constexpr auto Identity(){
			return matrix4x4(
				static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(1),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),static_cast<type>(1)
			);
		}

		//! @brief Create a scaling matrix.
		static inline constexpr auto Scaling(const type&_ScaleX,const type&_ScaleY,const type&_ScaleZ,const type&_ScaleW=1){
			return matrix4x4(
				_ScaleX             ,static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),_ScaleY             ,static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),_ScaleZ             ,static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),_ScaleW
			);
		}
		
		//! @brief Create a translation matrix.
		static inline constexpr auto Translation(const type&_X,const type&_Y,const type&_Z){
			return matrix4x4(
				static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),_X                 ,
				static_cast<type>(0),static_cast<type>(1),static_cast<type>(0),_Y                 ,
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(1),_Z                 ,
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),static_cast<type>(1)
			);
		}

		//! @brief Create a rotation matrix around the X axis.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto RotationX(const type&_Theta){
			auto _cos = _Math_Library::cos(_Theta), _sin = _Math_Library::sin(_Theta);
			return matrix4x4(
				_cos                ,-_sin               ,static_cast<type>(0),static_cast<type>(0),
				_sin                ,_cos                ,static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(1),static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),static_cast<type>(1)
			);
		}

		//! @brief Create a rotation matrix around the Y axis.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto RotationY(const type&_Theta){
			auto _cos = _Math_Library::cos(_Theta), _sin = _Math_Library::sin(_Theta);
			return matrix4x4(
				_cos                ,static_cast<type>(0),_sin                ,static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),
				-_sin               ,static_cast<type>(0),_cos                ,static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),static_cast<type>(1)
			);
		}

		//! @brief Create a rotation matrix around the Z axis.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto RotationZ(const type&_Theta){
			auto _cos = _Math_Library::cos(_Theta), _sin = _Math_Library::sin(_Theta);
			return matrix4x4(
				static_cast<type>(1),static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),
				static_cast<type>(0),_cos                ,-_sin               ,static_cast<type>(0),
				static_cast<type>(0),_sin                ,_cos                ,static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),static_cast<type>(1)
			);
		}

		//! @brief Create a rotation matrix.
		template <class _Math_Library = math::std_library>
		static inline constexpr auto Rotation(const type&_Yaw,const type&_Pitch,const type&_Roll){
			auto ax = _Math_Library::cos(_Yaw),   ay = _Math_Library::sin(_Yaw);
			auto bx = _Math_Library::cos(_Pitch), by = _Math_Library::sin(_Pitch);
			auto cx = _Math_Library::cos(_Roll),  cy = _Math_Library::sin(_Roll);
			return matrix4x4(
				ax * bx            , ax * by*cy - ay*cx , ax * by*cx + ay*cy ,static_cast<type>(0),
				ay * bx            , ay * by*cy + ax*cx , ay * by*cx - ax*cy ,static_cast<type>(0),
				-by                 , bx * cy            , bx * cx            ,static_cast<type>(0),
				static_cast<type>(0),static_cast<type>(0),static_cast<type>(0),static_cast<type>(1)
			);
		}
		
		//! @brief Get the transposed form of this matrix
		inline constexpr auto transpose(){return matrix4x4(
			m11, m21, m31, m41,
			m12, m22, m32, m42,
			m13, m23, m33, m43,
			m14, m24, m34, m44
		);}

		constexpr const auto&operator->()const{return reinterpret_cast<const xyzw<vector>*>(this);}
		constexpr auto&operator->(){return reinterpret_cast<xyzw<vector>*>(this);}

		constexpr auto getX()const{return vector(m11,m21,m31,m41);}
		constexpr auto getY()const{return vector(m12,m22,m32,m42);}
		constexpr auto getZ()const{return vector(m13,m23,m33,m43);}
		constexpr auto getW()const{return vector(m14,m24,m34,m44);}

		constexpr auto operator*(const vector&_Right)const{return vector(
			m11*_Right.x + m12*_Right.y + m13*_Right.z + m14*_Right.w,
			m21*_Right.x + m22*_Right.y + m23*_Right.z + m24*_Right.w,
			m31*_Right.x + m32*_Right.y + m33*_Right.z + m34*_Right.w,
			m41*_Right.x + m42*_Right.y + m43*_Right.z + m44*_Right.w
		);}
		constexpr auto operator()(const vector&_Right)const{return vector(
			m11*_Right.x + m12*_Right.y + m13*_Right.z + m14*_Right.w,
			m21*_Right.x + m22*_Right.y + m23*_Right.z + m24*_Right.w,
			m31*_Right.x + m32*_Right.y + m33*_Right.z + m34*_Right.w,
			m41*_Right.x + m42*_Right.y + m43*_Right.z + m44*_Right.w
		);}

		constexpr auto operator*(const matrix4x4&_Right)const{return matrix4x4(
			*this*_Right.getX(), *this*_Right.getY(), *this*_Right.getZ(), *this*_Right.getW()
		);}
		constexpr auto operator()(const matrix4x4&_Right)const{return matrix4x4(
			*this*_Right.getX(), *this*_Right.getY(), *this*_Right.getZ(), *this*_Right.getW()
		);}

	}; // Fission::base::matrix4x4

} // namespace Fission::base


/* ======================================== [Matrix Aliases] ======================================== */

// `float` will be the type used most often,
//	so we only need short-hand float variations

using m22 = math::matrix2x2<f32>;
using m23 = math::matrix2x3<f32>;
using m33 = math::matrix3x3<f32>;
using m34 = math::matrix3x4<f32>;
using m44 = math::matrix4x4<f32>;

__FISSION_END__

/**
 *	MIT License
 *
 *	Copyright (c) 2021-2023 lazergenixdev
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