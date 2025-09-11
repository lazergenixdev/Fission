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
#include <cstdint>     // -> sized integer types
#include <cmath>
#include <concepts>
#include <chrono>
#include <glm/glm.hpp> // TODO: may need to remove dependency
//#include <vector>      // TODO: remove

// --------------------------------------------------------------------------------
// Macro Helpers

#define MACRO_EXPAND(X) X
#define MACRO_STRING(...) #__VA_ARGS__
#define MACRO_STRING_EXPAND(X) MACRO_STRING(X)
#define MACRO_JOIN(A,B) A ## B
#define MACRO_JOIN_EXPAND(A,B) MACRO_JOIN(A,B)
#define MACRO_PRAGMA(X) _Pragma(#X)

#define BEGIN_NAMESPACE(name) namespace name {
#define END_NAMESPACE()       }

#define TEMP_VAR MACRO_JOIN_EXPAND(_, __LINE__)
#define NOT_USED(...) (void)sizeof(__VA_ARGS__)
#define global extern
#define internal static
#define local_persist static
#define forn(N) for (decltype(N) i = 0; i < (N); ++i)

// --------------------------------------------------------------------------------
// Source Location

struct source_location {
	const char* file;
	const char* function;
	int line;
};
#define CURRENT_LOCATION source_location{__FILE__, __PRETTY_FUNCTION__, __LINE__}

// --------------------------------------------------------------------------------
// Assertions

#if defined(DEBUG)
#	define ASSERT(cond) (cond ? 0 : ::os::fatal_error("Assertion Failed", #cond, CURRENT_LOCATION)) 
#else
#	define ASSERT(cond) (cond ? 0 : ::os::fatal_error("Assertion Failed", #cond, CURRENT_LOCATION)) 
#endif

// --------------------------------------------------------------------------------
// Compiler Detection

#if defined(__clang__)
#	define COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#   define COMPILER_GNU
#elif defined(_MSC_VER)
#   define COMPILER_MSVC
#else
#   pragma message("[Fission] Unable to detect compiler!")
#endif

// --------------------------------------------------------------------------------
// Normalize compiler intrinsics

#if defined(COMPILER_MSVC)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

// --------------------------------------------------------------------------------
// Disabling compiler warnings

#if defined(COMPILER_MSVC)
#   define DISABLE_WARNING(WARNINGS)  MACRO_PRAGMA(warning(disable: WARNINGS))
#   define DISABLE_ALL_WARNINGS_BEGIN MACRO_PRAGMA(warning(push, 0))
#   define DISABLE_ALL_WARNINGS_END   MACRO_PRAGMA(warning(pop))
#elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#   define DISABLE_WARNING(WARNING)   MACRO_PRAGMA(GCC diagnostic ignored WARNING)
#   define DISABLE_ALL_WARNINGS_BEGIN MACRO_PRAGMA(GCC diagnostic push) DISABLE_WARNING("-Weverything")
#   define DISABLE_ALL_WARNINGS_END   MACRO_PRAGMA(GCC diagnostic pop)
#endif

// --------------------------------------------------------------------------------
// Types

using byte = uint8_t;
using u8   = uint8_t;
using u16  = uint16_t;
using u32  = uint32_t;
using u64  = uint64_t;
using s8   = int8_t;
using s16  = int16_t;
using s32  = int32_t;
using s64  = int64_t;
using f32  = float;
using f64  = double;

// This is only for seeing the strings correctly in the debugger,
//    literally no other reason for this to be here :)
#if defined(__cpp_char8_t)
using c8  = char8_t;
using c16 = char16_t;
using c32 = char32_t;
#else
using c8  = u8;
using c16 = u16;
using c32 = u32;
#endif

#define FISSION_PRIMITIVE_ALIASES(BASE,NAME) \
using NAME ## s8  = BASE<s8>;                \
using NAME ## u8  = BASE<u8>;                \
using NAME ## s16 = BASE<s16>;               \
using NAME ## u16 = BASE<u16>;               \
using NAME ## s32 = BASE<s32>;               \
using NAME ## u32 = BASE<u32>;               \
using NAME ## s64 = BASE<s64>;               \
using NAME ## u64 = BASE<u64>;               \
using NAME ## f32 = BASE<f32>;               \
using NAME ## f64 = BASE<f64>

// --------------------------------------------------------------------------------
// Template Meta Programming

BEGIN_NAMESPACE(meta)

template <typename>
static constexpr bool always_false = false; // Thanks C++, very cool

// Base
template <int, typename...>
struct _type_at { using type = void; };

// Single Type
template <int i, typename T>
struct _type_at<i, T> { using type = T; };

// Two or more Types
template <int i, typename T, typename...Rest>
struct _type_at<i, T, Rest...> {
	using type = std::conditional_t<i <= 0, T, typename _type_at<i - 1, Rest...>::type>;
};

template <int i, typename...T>
using type_at = typename _type_at<i, T...>::type;


// Base
template <int, typename...>
struct _size_of_n {
	static constexpr u32 value = 0u;
};

// Two or more Types
template <int i, typename T, typename...Rest>
struct _size_of_n<i, T, Rest...> {
	static constexpr u32 value = (i > 0) ? (sizeof(T) + _size_of_n<i - 1, Rest...>::value) : 0u;
};

// Single Type
template <int i, typename T>
struct _size_of_n<i, T> {
	static constexpr u32 value = (i > 0) ? sizeof(T) : 0u;
};

template <int i, typename...T>
static constexpr u32 size_of_n = _size_of_n<i, T...>::value;

template <typename...T>
static constexpr u32 size_of = _size_of_n<sizeof...(T), T...>::value;

END_NAMESPACE()

// --------------------------------------------------------------------------------
// Constants

namespace fission
{
	// Set Variable only for a scope
	template <typename T>
	struct scoped_set_variable
	{
		T original;
		T& dst;
		~scoped_set_variable() { dst = original; }
	};
#	define scoped_set(dst, original) auto TEMP_VAR = scoped_set_variable{ dst, dst = original }
}

// --------------------------------------------------------------------------------
// Constants

#define PI   (3.1415926535897932384626433)
#define TAU  (6.2831853071795864769252867)

// --------------------------------------------------------------------------------
// Conversion -> Bytes

inline constexpr unsigned long long operator"" _KiB(unsigned long long KiB)
{
    return KiB * 1024ULL;
}
inline constexpr unsigned long long operator"" _MiB(unsigned long long MiB)
{
    return MiB * 1024ULL * 1024ULL;
}
inline constexpr unsigned long long operator"" _GiB(unsigned long long GiB)
{
    return GiB * 1024ULL * 1024ULL * 1024ULL;
}

// --------------------------------------------------------------------------------
// Math

namespace math
{
	template <typename type, typename from>
	inline constexpr type max(type a, from b)
    {
        auto const _b = static_cast<type>(b);
        return (a > _b) ? a : _b;
	}

    template <typename type, typename from>
    inline constexpr type min(type a, from b)
    {
        auto const _b = static_cast<type>(b);
        return (a < _b) ? a : _b;
    }
	
	template <typename T, typename F>
	inline constexpr T lerp(T const& left, T const& right, F x)
    {
		return left * ((F)1 - x) + right * x;
	}

    template <typename T>
    static inline constexpr T lerp_speed(T dt, T speed)
    {
        return T(1.0) - std::pow(T(0.5), dt * speed);
    }

    template <typename T>
    static inline constexpr T exp_update(T current, T target, T dt, T speed)
    {
        return lerp(current, target, lerp_speed(dt, speed));
    }

    template <typename T>
    static constexpr T floor(T const& x)
    {
        static_assert(std::is_floating_point_v<T>);
        T const q = static_cast<T>(static_cast<int>(x));
        return q - static_cast<T>(x < 0);
    }

    template <typename T>
    static constexpr T mod(T const& x, T const& y)
    {
        // This is slightly faster than glm::mod's `a - b * floor(a / b)`
        // Tested with clang -O2 and with clang -O3
        if constexpr (std::is_floating_point_v<T>) {
            T const q = static_cast<T>(static_cast<int>(x / y));
            T const f = static_cast<T>((y < 0)^(x < 0)); // fix for q
            // x = q * y + r   with  0 <= r < y (generally)
            return x - (q - f) * y;
        }
    }

    struct std_library
    {
        template <typename T>
        static inline constexpr auto sin(T const&x) { return std::sin(x); }

        template <typename T>
        static inline constexpr auto cos(T const&x) { return std::cos(x); }

        template <typename T>
        static inline constexpr auto sqrt(T const&x) { return std::sqrt(x); }
    };

} // ::math

// --------------------------------------------------------------------------------
// Math operators helpers

#define FISSION_IMPLEMENT_OPERATORS_2(BASE, TYPE, X, Y) \
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

#define FISSION_IMPLEMENT_OPERATOR_MULTIPLY_2(BASE, TYPE, X, Y) \
inline constexpr auto operator*(TYPE const&left,BASE const&right){return BASE{left*right.X,left*right.Y};}

#define FISSION_IMPLEMENT_OPERATOR_DIVISION_2(BASE, TYPE, X, Y) \
inline constexpr auto operator/(TYPE const&left,BASE const&right){return BASE{left/right.X,left/right.Y};}

#define FISSION_IMPLEMENT_OPERATOR_DOT_2(BASE, X, Y) \
inline constexpr auto dot(BASE const&left,BASE const&right){return left.X*right.X+left.Y*right.Y;}

#define FISSION_IMPLEMENT_OPERATORS_3(BASE, TYPE, X, Y, Z) \
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

#define FISSION_IMPLEMENT_OPERATOR_MULTIPLY_3(BASE, TYPE, X, Y, Z) \
inline constexpr auto operator*(TYPE const&left,BASE const&right){return BASE{left*right.X,left*right.Y,left*right.Z};}

#define FISSION_IMPLEMENT_OPERATOR_DIVISION_3(BASE, TYPE, X, Y, Z) \
inline constexpr auto operator/(TYPE const&left,BASE const&right){return BASE{left/right.X,left/right.Y,left/right.Z};}

#define FISSION_IMPLEMENT_OPERATOR_DOT_3(BASE, X, Y, Z) \
inline constexpr auto dot(BASE const&left,BASE const&right){return left.X*right.X+left.Y*right.Y+left.Z*right.Z;}

#define FISSION_IMPLEMENT_OPERATORS_4(BASE, TYPE, X, Y, Z, W) \
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

#define FISSION_IMPLEMENT_OPERATOR_MULTIPLY_4(BASE, TYPE, X, Y, Z, W) \
inline constexpr BASE operator*(TYPE const&left,BASE const&right){return {left*right.X,left*right.Y,left*right.Z,left*right.W};}

#define FISSION_IMPLEMENT_OPERATOR_DIVISION_4(BASE, TYPE, X, Y, Z, W) \
inline constexpr BASE operator/(TYPE const&left,BASE const&right){return {left/right.X,left/right.Y,left/right.Z,left/right.W};}

#define FISSION_IMPLEMENT_OPERATOR_DOT_4(BASE, X, Y, Z, W) \
inline constexpr auto dot(BASE const&left,BASE const&right){return left.X*right.X+left.Y*right.Y+left.Z*right.Z+left.W*right.W;}

// --------------------------------------------------------------------------------
// Vectors

namespace fission
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
		constexpr type len()const{return std::sqrt(this->x*this->x+this->y*this->y);}

		//! @brief Get the normal vector for this vector.
		//! @warning UNDEFINED FOR VECTOR {0,0}
		constexpr vector2 norm()const{auto k=len();return vector2(this->x/k,this->y/k);}

		//! @brief Transform vector so that it has a length of one.
		//! @warning UNDEFINED FOR VECTOR {0,0}
		//! @return Reference to this vector.
		constexpr vector2&normalize(){auto k=len();this->x/=k,this->y/=k;return*this;}

		inline constexpr bool operator==(vector2 const&) const = default;

		FISSION_IMPLEMENT_OPERATORS_2(vector2, type, x, y)

	}; // vector2

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
		constexpr type len()const{return std::sqrt(this->x*this->x+this->y*this->y+this->z*this->z);}

		//! @brief Get the normal vector for this vector.
		//! @warning UNDEFINED FOR VECTOR {0,0,0}
		constexpr vector3 norm()const{auto k=len();return vector3(this->x/k,this->y/k,this->z/k);}

		//! @brief Transform vector so that it has a length of one.
		//! @warning UNDEFINED FOR VECTOR {0,0,0}
		//! @return Reference to this vector.
		constexpr vector3&normalize(){auto k=len();this->x/=k,this->y/=k,this->z/=k;return*this;}


		inline constexpr bool operator==(vector3 const&) const = default;

		FISSION_IMPLEMENT_OPERATORS_3(vector3, type, x, y, z)

	}; // vector3

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
		constexpr type len()const{return std::sqrt(this->x*this->x+this->y*this->y+this->z*this->z+this->w*this->w);}

		//! @brief Get the normal vector for this vector.
		//! @warning UNDEFINED FOR VECTOR {0,0,0,0}
		constexpr vector4 norm()const{auto k=len();return vector4(this->x/k,this->y/k,this->z/k,this->w/k);}

		//! @brief Transform vector so that it has a length of one.
		//! @warning UNDEFINED FOR VECTOR {0,0,0,0}
		//! @return Reference to this vector.
		constexpr vector4&normalize(){auto k=len();this->x/=k,this->y/=k,this->z/=k,this->w/=k;return*this;}


		constexpr bool operator==(vector4 const&) const = default;

		FISSION_IMPLEMENT_OPERATORS_4(vector4, type, x, y, z, w)

	}; // vector4

	template<typename T> using v2 = vector2<T>;
	template<typename T> using v3 = vector3<T>;
	template<typename T> using v4 = vector4<T>;

	FISSION_PRIMITIVE_ALIASES(vector2, v2);
	FISSION_PRIMITIVE_ALIASES(vector3, v3);
	FISSION_PRIMITIVE_ALIASES(vector4, v4);

	// Default vector types
	using vec2 = vector2<f32>;
	using vec3 = vector3<f32>;
	using vec4 = vector3<f32>;
} // fission

template <typename T> FISSION_IMPLEMENT_OPERATOR_MULTIPLY_2(fission::vector2<T>, T, x, y)
template <typename T> FISSION_IMPLEMENT_OPERATOR_MULTIPLY_3(fission::vector3<T>, T, x, y, z)
template <typename T> FISSION_IMPLEMENT_OPERATOR_MULTIPLY_4(fission::vector4<T>, T, x, y, z, w)

template <typename T> FISSION_IMPLEMENT_OPERATOR_DIVISION_2(fission::vector2<T>, T, x, y)
template <typename T> FISSION_IMPLEMENT_OPERATOR_DIVISION_3(fission::vector3<T>, T, x, y, z)
template <typename T> FISSION_IMPLEMENT_OPERATOR_DIVISION_4(fission::vector4<T>, T, x, y, z, w)

template <typename T> FISSION_IMPLEMENT_OPERATOR_DOT_2(fission::vector2<T>, x, y)
template <typename T> FISSION_IMPLEMENT_OPERATOR_DOT_3(fission::vector3<T>, x, y, z)
template <typename T> FISSION_IMPLEMENT_OPERATOR_DOT_4(fission::vector4<T>, x, y, z, w)

// --------------------------------------------------------------------------------
// Result Values

namespace fission
{
	enum Result: s32 {
		Success   = 0,
		Failed    = 1,
	};
}

// --------------------------------------------------------------------------------
// Arena Allocators

namespace fission
{
	struct Arena
	{
		void* start        {};
		size_t allocated   {};
		size_t capacity    {};

		template <typename T>
		struct Temp_Array
		{
			size_t count;
			T* data;
			Arena& arena;
			size_t checkpoint;

			inline constexpr T& operator[](size_t i) const { return (T&)data[i]; }
			inline constexpr T* begin() const { return data; }
			inline constexpr T* end() const { return data + count; }

			inline ~Temp_Array() {
				arena.allocated = checkpoint;
			}
		};

		auto create(size_t max_size) -> Result;
		void destroy();
		auto alloc(size_t size) -> void*;
		
		inline void reset() { allocated = 0; }

		template <typename T>
		inline auto temp_array(size_t count) -> Temp_Array<T>
		{
			auto checkpoint = allocated;
			auto ptr = alloc<T>(count);
			return { count, ptr, *this, checkpoint };
		}

		// Prevent future bugs from C++'s shitty template deduction
		inline void push_byte(byte b) { push<byte>(b); }

		template <typename T>
		inline auto alloc(size_t count) -> T* {
			return reinterpret_cast<T*>(alloc(count*sizeof(T)));
		}
		//! TODO: OPTIMIZATION: Add alignment
		template <typename T=void>
		inline auto next_ptr() -> T* {
			return reinterpret_cast<T*>(reinterpret_cast<byte*>(start) + allocated);
		}
		inline auto remaining() -> size_t {
			return capacity - allocated;
		}
		template <typename T>
		inline auto push(T* data, size_t count) -> T* {
			auto ptr = alloc<T>(count);
			memcpy((void*)(ptr), data, count * sizeof(T));
			return ptr;
		}
		template <typename T>
		inline auto push(T obj) -> T* {
			auto ptr = alloc<T>(1);
			memcpy((void*)(ptr), &obj, sizeof(T));
			return ptr;
		}
	};

	//! NOTE: Will invalidate pointers into arena!
	struct Dynamic_Arena: public Arena
	{
		auto create(size_t max_size) -> Result;
		void destroy();
		auto alloc(size_t size) -> void*;
	};

	global Arena scratch_arena;
	extern Arena& temp_arena();
}

// --------------------------------------------------------------------------------
// Strings (UTF-8 encoding)

namespace fission
{
	struct string
	{
		size_t count = 0;
		c8*    data  = nullptr;

		inline constexpr string() = default;

		template <size_t literal_size>
		inline constexpr string(char const(&literal)[literal_size])
		:	count(literal_size-1), data((c8*)literal)
		{}

		template <size_t buffer_size>
		inline constexpr string(char (&buffer)[buffer_size])
		:	count(buffer_size), data(reinterpret_cast<c8*>(buffer))
		{}

		template <typename string_type>
		inline constexpr string(string_type const& s)
		:   count(s.size()), data((c8*)s.data())
		{}

		inline constexpr string(void const* ptr, u64 size)
		:	count(size), data((c8*)ptr)
		{}

		template <size_t buffer_size>
		inline constexpr string from_buffer(char (&buffer)[buffer_size]) {
			return {buffer, buffer_size};
		}
		
		// Conversion for std-string-like types
		template <typename T>
		inline T as() const { return {(char*)data, count}; }
		
		//! NOTE: make sure string IS null-terminated yourself
		inline const char* cstr() const { return (char*)data; }

		inline constexpr string substr(u64 offset, u64 max_count = 0xFFFFFFFF) const {
			return {data + offset, math::min(count - offset, max_count)};
		}

		inline constexpr bool is_empty() const { return count == 0; }

		inline constexpr c8* begin() const { return data; }
		inline constexpr c8* end  () const { return data + count; }
	};
	
	// UTF-16
	struct string_u16
	{
		size_t count = 0;
		c16*   data  = nullptr;
		
		//! NOTE: make sure string IS null-terminated yourself
		inline const wchar_t* wstr() const { return (wchar_t*)data; }
	};

	inline void format_single(Arena& arena, string s)
	{
		arena.push(s.data, s.count);
	}
	inline void format_single(Arena& arena, const char* cstring)
	{
		arena.push(cstring, strlen(cstring));
	}
	template <size_t size>
	inline void format_single(Arena& arena, const char (&string_literal)[size])
	{
		arena.push(const_cast<char*>(string_literal), size - 1);
	}
}

// --------------------------------------------------------------------------------
// Formatting

namespace fission
{
	template <std::integral T>
	inline void format_single(Arena& arena, const T value)
	{
		if (value == 0) { arena.push('0'); return; }
		if constexpr (std::is_same<T, bool>::value)
		{
			if (value) arena.push("true", 4);
			else       arena.push("false", 5);
		}
		else if constexpr (std::is_signed<T>::value)
		{
			if (value < 0) arena.push('-');
			format_single(arena, u64(value < 0? -value : value));
		}
		else if constexpr (std::is_unsigned<T>::value)
		{
			u64 x = value, p = 10000000000000000000ULL;
			while (p != 0) {
				u64 d = x / p;
				if (d != 0 || x == 0) {
					arena.push_byte(byte('0' + d));
					x -= d * p;
				}
				p /= 10;
			}
		}
	}

	//! TODO: use current context arena
	template <typename...T>
	inline auto format(Arena& arena, T&&...args) -> string
	{
		auto buffer = arena.next_ptr<char>();
		(format_single(arena, std::forward<T>(args)), ...);
		return string(buffer, static_cast<u64>(arena.next_ptr<char>() - buffer));
	}

	namespace formatting
	{
		template <typename T>
		struct padded_object
		{
			T object;
			size_t padding;
		};

		template <typename T>
		auto pad(T const& object, size_t padding) -> padded_object<T>
		{
			return {object, padding};
		}

		struct {} null;
	}

	template <typename T>
	inline void format_single(Arena& arena, formatting::padded_object<T> const& padded_object)
	{
		size_t start = arena.allocated;
		format_single(arena, padded_object.object);
		size_t length = arena.allocated - start;
		if (length < padded_object.padding)
		{
			memset(arena.next_ptr(), ' ', padded_object.padding - length);
			arena.allocated += padded_object.padding - length;
		}
	}
	
	inline void format_single(Arena& arena, decltype(formatting::null))
	{
		arena.push_byte(0);
	}
}

// --------------------------------------------------------------------------------
// Ranges

namespace fission
{
	template <typename type>
	struct range
	{
		type low;  // lower bound of the range
		type high; // upper bound of the range


		//! @brief Create null range: {0,0}.
		constexpr range():low(static_cast<type>(0)),high(static_cast<type>(0)){}

		//! @brief Create range from a single value. [0, n]
		constexpr explicit range(type const& n):low(static_cast<type>(0)),high(n){}

		//! @brief Create range from a low and high value, where lo < hi.
		constexpr range(type const& lo,type const& hi):low(lo),high(hi){}

		//! @brief Converts range from another type.
		template <typename from> explicit
		constexpr range(range<from> const& r):low(static_cast<type>(r.low)),high(static_cast<type>(r.high)){}

		//! @brief Creates valid range from two values A and B.
		static inline constexpr range create(type const& A,type const& B){return(A>B)?range(B,A):range(A,B);}

		//! @brief Creates valid range from center and span.
		static inline constexpr range from_center(type const&center,type const&span){type d=span/static_cast<type>(2);return range(center-d, center+d);}


		// Range Functions

		//!! @brief Clamps a value to this range.
		inline constexpr type clamp(type const&x)const{if(x<this->low)return this->low;if(x>this->high)return this->high;return x;}

		//! @brief Checks whether value is contained within open range: ( low, high ).
		inline constexpr bool operator()(type const&x)const{return(x>this->low)&&(x<this->high);}

		//! @brief Checks whether value is contained within closed range: [ low, high ].
		inline constexpr bool operator[](type const&x)const{return(x>=this->low)&&(x<=this->high);}

		//! @brief Checks whether value is contained within ( low, high ].
		inline constexpr bool closed_upper(type const&x)const{return(x>this->low)&&(x<=this->high);}
		
		//! @brief Checks whether value is contained within [ low, high ).
		inline constexpr bool closed_lower(type const&x)const{return(x>=this->low)&&(x<this->high);}

		//! @brief Get the distance between the two end points of this range.
		inline constexpr type distance()const{return this->high-this->low;}
		inline constexpr type difference()const{return this->high-this->low;}

		//! @brief Get the value of the center.
		inline constexpr type center()const{return(this->high+this->low)/static_cast<type>(2);}
		inline constexpr type average()const{return(this->high+this->low)/static_cast<type>(2);}

		//! @brief Determine whether the high value is greater than or equal to the low value.
		//! @note foreach loops will only function when high >= low (unless a negative step value is used)
		inline constexpr bool valid()const{return this->high>=this->low;}


		// Modification Functions
		
		//! @brief Get an Expanded range.
		inline constexpr auto expanded(type const&_dx)const{return range(this->low-_dx,this->high+_dx);}

		//! @brief Expands this range
		inline constexpr auto&expand(type const&_dx){this->low-=_dx,this->high+=_dx;return*this;}

		//! @brief Get a Scaled range from center.
		inline constexpr auto scaled(type const&scale)const{
			auto center=(this->high+this->low)/static_cast<type>(2),
				d=scale*(this->high-this->low)/static_cast<type>(2); 
			return range(center-d,center+d);
		}

		//! @brief Scales this range from center.
		inline constexpr auto&scale(type const&scale){
			auto center=(this->high+this->low)/static_cast<type>(2),
				d=scale*(this->high-this->low)/static_cast<type>(2); 
			this->low=center-d,this->high=center+d;return*this;
		}

		// Extras
	private:

		struct range_iterator
		{
			type value;

			constexpr bool operator!=(range_iterator const& r)const{return value<r.value;}
			constexpr type operator*()const{return value;}
			constexpr range_iterator& operator++(){++value;return*this;}
		};

		struct step_range_iterator
		{
			type value, step;

			constexpr bool operator!=(range_iterator const& r)const{return value<r.value;}
			constexpr type operator*()const{return value;}
			constexpr step_range_iterator& operator++(){value+=step;return*this;}
		};

		struct stepped_range
		{
			type low, high, step;

			inline constexpr auto begin()const{return step_range_iterator{low,step};}
			inline constexpr auto end()const{return range_iterator{high};}
		};

	public:

		inline constexpr auto step(type const& step)const{return stepped_range{this->low, this->high, step};}

		inline constexpr auto begin()const{return range_iterator{this->low};}
		inline constexpr auto end()const{return range_iterator{this->high};}


		// Operators

		/*! @brief Check if the ranges are equal. */
		constexpr bool operator==(range const&) const = default;

		//! @brief Get a range scaled from zero.
		constexpr auto operator*(type const&scale)const{return range(this->low*scale,this->high*scale);}

		//! @brief Scale this range from zero.
		constexpr auto&operator*=(type const&scale){this->low*=scale,this->high*=scale;return*this;}
		constexpr auto&operator/=(type const&scale){this->low/=scale,this->high/=scale;return*this;}

		// vvv Shift Operators vvv

		constexpr auto operator+(type const&shift)const{return range(this->low+shift,this->high+shift);}
		constexpr auto operator-(type const&shift)const{return range(this->low-shift,this->high-shift);}

		constexpr auto&operator+=(type const&shift){this->low+=shift,this->high+=shift;return*this;}
		constexpr auto&operator-=(type const&shift){this->low-=shift,this->high-=shift;return*this;}

	}; // range
	
	FISSION_PRIMITIVE_ALIASES(range, range);
}

// --------------------------------------------------------------------------------
// Rectangles (rect)

namespace fission
{
	//! @brief Structure defining a rectangle containing min and mox for X and Y.
	template <typename T>
	struct rect
	{
		using type = T;
		using vector = vector2<T>;

		range<T> x, y;


		//! @brief Create a null rect.
		constexpr rect():x(),y(){}

		//! @brief Create rect from 4 values: left, right, top, and bottom.
		constexpr rect(type const&x_low,type const&x_high,type const&y_low,type const&y_high):x(x_low,x_high),y(y_low,y_high){}
			
		//! @brief Create rect from two ranges, for the X range and Y range.
		constexpr rect(const range<T>&X,const range<T>&Y):x(X),y(Y){}

		//! @brief Create a valid rect from two vectors.
		constexpr rect(vector const&a,vector const&b):x(range<T>::create(a.x,b.x)),y(range<T>::create(a.y,b.y)){}

		//! @brief Create rect from another rect with a different type.
		template <typename from> explicit
		constexpr rect(rect<from> const& r):x(static_cast<range<T>>(r.x)),y(static_cast<range<T>>(r.y)){}


		//! @brief Create a rect from a top-left position and a size.
		static inline constexpr rect from_topleft(type const&left,type const&top,type const&width,type const&height)
		{
			return rect(left,left+width,top,top+height);
		}
		static inline constexpr rect from_topleft(type const&left,type const&top,vector const&size)
		{
			return rect(left,left+size.x,top,top+size.y);
		}
		static inline constexpr rect from_topleft(vector const&topleft,type const&width,type const&height)
		{
			return rect(topleft.x,topleft.x+width,topleft.y,topleft.y+height);
		}
		static inline constexpr rect from_topleft(vector const&topleft,vector const&size)
		{
			return rect(topleft.x,topleft.x+size.x,topleft.y,topleft.y+size.y);
		}
		static inline constexpr rect from_topleft(type const&width,type const&height)
		{
			return rect(static_cast<type>(0),width,static_cast<type>(0),height);
		}
		static inline constexpr rect from_topleft(vector const&size)
		{
			return rect(static_cast<type>(0),size.x,static_cast<type>(0),size.y);
		}


		//! @brief Create a rect from an center position and a size.
		static inline constexpr rect from_center(type const&center_x,type const&center_y,type const&width,type const&height)
		{
			const auto dx = width / static_cast<type>(2), dy = height / static_cast<type>(2);
			return rect(center_x-dx,center_x+dx,center_y-dy,center_y+dy);
		}
		static inline constexpr rect from_center(type const&center_x,type const&center_y,vector const&size)
		{
			const auto dx = size.x / static_cast<type>(2), dy = size.y / static_cast<type>(2);
			return rect(center_x-dx,center_x+dx,center_y-dy,center_y+dy);
		}
		static inline constexpr rect from_center(vector const&center,type const&width,type const&height)
		{
			const auto dx = width / static_cast<type>(2), dy = height / static_cast<type>(2);
			return rect(center.x-dx,center.x+dx,center.y-dy,center.y+dy);
		}
		static inline constexpr rect from_center(vector const&center,vector const&size)
		{
			const auto dx = size.x / static_cast<type>(2), dy = size.y / static_cast<type>(2);
			return rect(center.x-dx,center.x+dx,center.y-dy,center.y+dy);
		}
		static inline constexpr rect from_center(type const& width,type const& height)
		{
			const auto dx = width / static_cast<type>(2), dy = height / static_cast<type>(2);
			return rect(-dx,dx,-dy,+dy);
		}
		static inline constexpr rect fromcenter(vector const&size)
		{
			const auto dx = size.x / static_cast<type>(2), dy = size.y / static_cast<type>(2);
			return rect(-dx,dx,-dy,+dy);
		}


		//! @brief Create a rect from a Windows RECT.
		template <typename win32_rect>
		static inline constexpr rect from_win32(win32_rect const& r)
		{
			return {r.left, r.right, r.top, r.bottom};
		}

		// Getters

		inline constexpr auto left()  const{return this->x.low ;}
		inline constexpr auto right() const{return this->x.high;}
		inline constexpr auto top()   const{return this->y.low ;}
		inline constexpr auto bottom()const{return this->y.high;}

		inline constexpr auto top_left() const{return vector(this->x.low, this->y.low );}
		inline constexpr auto top_right()const{return vector(this->x.high,this->y.low );}
		inline constexpr auto bot_left() const{return vector(this->x.low, this->y.high);}
		inline constexpr auto bot_right()const{return vector(this->x.high,this->y.high);}

		inline constexpr auto width()const{return this->x.high-this->x.low;}
		inline constexpr auto height()const{return this->y.high-this->y.low;}
		inline constexpr auto size()const{return vector(this->x.high-this->x.low,this->y.high-this->y.low);}
		inline constexpr auto center()const{auto c=vector(this->x.low+this->x.high,this->y.low+this->y.high);return c/static_cast<type>(2);}


		// Modification Functions

		//! @brief Get a rect where the X and Y values are fliped.
		constexpr rect fliped(){return rect(this->y,this->x);}

		//! @brief flips the X and Y ranges
		constexpr rect & flip() { 
			std::swap(this->x,this->y);
			return *this;
		}

		//! @brief Get a rect that is expanded in all directions
		constexpr rect expanded(type const&expand)const{return rect(
			this->x.low-expand,this->x.high+expand,this->y.low-expand,this->y.high+expand
		);}

		//! @brief Expand this rect in all directions
		constexpr rect expand(type const&expand){
			this->x.low-=expand,this->y.low-=expand;
			this->x.high+=expand,this->y.high+=expand;
			return *this;
		}

		//! @brief Get a rect that is Scaled from the center
		constexpr rect scaled(type const& scale)const{
			return rect(this->y.scaled(scale),this->y.scaled(scale));
		}

		//! @brief Scale this rect from the center
		constexpr rect scale(type const& scale){
			auto c=center(),
				 d=scale*vector(this->x.high-this->x.low,this->y.high-this->y.low)/static_cast<type>(2);
			this->x.low =c.x-d.x,this->y.low =c.y-d.y;
			this->x.high=c.x+d.x,this->y.high=c.y+d.y;
			return *this;
		}

		//! @brief Shift by an offset vector
		inline constexpr rect operator+(vector const& offset)const{return {this->x.low+offset.x,this->x.high+offset.x,this->y.low+offset.y,this->y.high+offset.y}; }

		// Rect Functions

		constexpr bool operator()(vector const&p)const{return x(p.x)&&y(p.y);}
		constexpr bool operator[](vector const&p)const{return x[p.x]&&y[p.y];}
		
		constexpr bool operator[](rect const& r)const{return x[r.x.low]&&x[r.x.high]&&y[r.y.low]&&y[r.y.high];}

		//! @brief Check if position is within [xmin,xmax) and [ymin,ymax).
		constexpr bool closed_lower(vector const&p)const{return x.closed_lower(p.x)&&y.closed_lower(p.y);}

		//! @brief Check if position is within (xmin,xmax] and (ymin,ymax].
		constexpr bool closed_upper(vector const&p)const{return x.closed_upper(p.x)&&y.closed_upper(p.y);}

		inline constexpr auto clamp(vector const& p)const{return vector(x.clamp(p.x),y.clamp(p.y));}

		inline constexpr rect operator*(type   const& r)const{return {this->x.low*r,this->x.high*r,this->y.low*r,this->y.high*r};}
		inline constexpr rect operator*(vector const& r)const{return {this->x.low*r.x,this->x.high*r.x,this->y.low*r.y,this->y.high*r.y};}

		inline constexpr rect operator/(type   const& r)const{return {this->x.low/r,this->x.high/r,this->y.low/r,this->y.high/r};}
		inline constexpr rect operator/(vector const& r)const{return {this->x.low/r.x,this->x.high/r.x,this->y.low/r.y,this->y.high/r.y};}

		inline constexpr bool valid()const{return x.valid()&&y.valid();}

		constexpr bool operator==(rect const&) const = default;

	}; // rect

	FISSION_PRIMITIVE_ALIASES(rect, r);
}

// --------------------------------------------------------------------------------
// Arrays

namespace fission
{
	template <typename T, u32 count>
	u32 array_count(T (&)[count]) { return count; }

	template <typename T>
	struct array
	{
		size_t count;
		T* data;

		inline constexpr T* begin() const { return const_cast<T*>(data); }
		inline constexpr T* end() const { return const_cast<T*>(data + count); }
	};
}

/**
 *	MIT License
 *
 *	Copyright (c) 2025 lazergenixdev
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
