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
 * 
 * INDEX:
 *    [Color Types]
 *    [Known Colors]
 *    [Color Definitions]
 *    [Color Implementation]
 *       (Color Conversions)
 *       (Trivial Conversions)
 */
#pragma once
#include <Fission/config.hpp>
#include <Fission/base/math/library.hpp> // math constants & fp_mod
#include "impl/operators.hpp"            // math operators for `rgb` & `rgba`
#include <cstdlib>

#define _FISSION_IMPLEMENT_BASIC_COLOR(TYPE, X1, X2, X3) \
type X1, X2, X3; \
constexpr TYPE():X1(static_cast<type>(0)),X2(static_cast<type>(0)),X3(static_cast<type>(0)){} \
constexpr TYPE(type const& X1,type const& X2,type const& X3):X1(X1),X2(X2),X3(X3){} \
constexpr bool operator==(TYPE const& _Right)const{return(X1==_Right.X1)&&(X2==_Right.X2)&&(X3==_Right.X3);}

#define _FISSION_IMPLEMENT_BASIC_COLORA(TYPE, X1, X2, X3, X4) \
type X1, X2, X3, X4; \
constexpr TYPE():X1(static_cast<type>(0)),X2(static_cast<type>(0)),X3(static_cast<type>(0)),X4(static_cast<type>(0)){} \
constexpr TYPE(type const& X1,type const& X2,type const& X3,type const& X4 = impl::max_color_value<type>):X1(X1),X2(X2),X3(X3),X4(X4){} \
constexpr bool operator==(TYPE const& _Right)const{return(X1==_Right.X1)&&(X2==_Right.X2)&&(X3==_Right.X3)&&(X4==_Right.X4);}

__FISSION_BEGIN__


///////////////////////////////////////////////////////////////////////////////
// [Color Types]
///////////////////////////////////////////////////////////////////////////////

struct rgb8;
struct rgba8;

struct rgb;
struct rgba;

struct hsv; // HSV / HSB
struct hsva;

struct hsl;
struct hsla;

struct lab; // CIELAB
struct laba; // CIELAB

struct cmyk;

using color_t = u32;
using color = rgba; // Default Color Type



///////////////////////////////////////////////////////////////////////////////
// [Known Colors]
///////////////////////////////////////////////////////////////////////////////

namespace colors
{
	enum known : color_t
	{
#		define X(NAME,VALUE) NAME = VALUE,
#		include "impl/colors.inl"
#		undef X
	}; // enum known

	template <color_t HEX>
	static constexpr known make = static_cast<known>( (HEX << 8) | 0xFF );

	// Generate a random color, meant only for development
	inline known random() {
		float r = float(rand()) / float(RAND_MAX);
		float g = float(rand()) / float(RAND_MAX);
		float b = float(rand()) / float(RAND_MAX);
		auto value =                color_t(r * 255.0f);
			 value = (value << 8) | color_t(g * 255.0f);
			 value = (value << 8) | color_t(b * 255.0f);
		return static_cast<known>( (value << 8) | 0xFF );
	}
}

namespace impl
{
	template <typename T> struct _max_color {};

	template <> struct _max_color<float> {
		static constexpr float value = 1.0f;
	};

	template <> struct _max_color<u8> {
		static constexpr u8 value = 255;
	};

	template <typename T>
	static constexpr T max_color_value = _max_color<T>::value;
}



///////////////////////////////////////////////////////////////////////////////
// [Color Definitions]
///////////////////////////////////////////////////////////////////////////////

struct rgb8
{
	using type = u8;

	_FISSION_IMPLEMENT_BASIC_COLOR(rgb8, r, g, b);

	constexpr rgb8(colors::known const& c):
		r(static_cast<type>(c >> 24)),
		g(static_cast<type>(c >> 16)),
		b(static_cast<type>(c >>  8))
	{}

	constexpr operator rgb() const;
};

struct rgba8
{
	using type = u8;

	_FISSION_IMPLEMENT_BASIC_COLORA(rgba8, r, g, b, a);
	
	constexpr rgba8(
		rgb8 const& _Color,
		type const& _Alpha = impl::max_color_value<type>
	):
		r(_Color.r), g(_Color.g), b(_Color.b), a(_Alpha)
	{}

	constexpr rgba8(colors::known const& c):
		r(static_cast<type>(c >> 24)),
		g(static_cast<type>(c >> 16)),
		b(static_cast<type>(c >>  8)),
		a(static_cast<type>(c      ))
	{}

	constexpr operator rgba() const;
};

struct rgb
{
	using type = float;

	_FISSION_IMPLEMENT_BASIC_COLOR(rgb, r, g, b);
	
	constexpr rgb(colors::known const& c):
		r(static_cast<type>((c >> 24) & 0xFF) / 0xFFp0f),
		g(static_cast<type>((c >> 16) & 0xFF) / 0xFFp0f),
		b(static_cast<type>((c >>  8) & 0xFF) / 0xFFp0f)
	{}

	constexpr operator rgb8() const;
	constexpr operator hsv() const;
	constexpr operator hsl() const;

	_FISSION_IMPLEMENT_OPERATORS_3(rgb, type, r, g, b);
};
_FISSION_IMPLEMENT_OPERATOR_MULTIPLY_3(rgb, rgb::type, r, g, b)

struct rgba
{
	using type = float;

	_FISSION_IMPLEMENT_BASIC_COLORA(rgba, r, g, b, a);
	
	constexpr rgba(
		rgb  const& _Color,
		type const& _Alpha = impl::max_color_value<type>
	):
		r(_Color.r), g(_Color.g), b(_Color.b), a(_Alpha)
	{}
	
	constexpr rgba(colors::known const& c):
		r(static_cast<type>((c >> 24) & 0xFF) / 0xFFp0f),
		g(static_cast<type>((c >> 16) & 0xFF) / 0xFFp0f),
		b(static_cast<type>((c >>  8) & 0xFF) / 0xFFp0f),
		a(static_cast<type>((c      ) & 0xFF) / 0xFFp0f)
	{}

	constexpr operator rgba8() const;
	constexpr operator hsva() const;
	constexpr operator hsla() const;

	_FISSION_IMPLEMENT_OPERATORS_4(rgba, type, r, g, b, a);
};
_FISSION_IMPLEMENT_OPERATOR_MULTIPLY_4(rgba, rgba::type, r, g, b, a)

struct hsv
{
	using type = float;

	_FISSION_IMPLEMENT_BASIC_COLOR(hsv, h, s, v);

	constexpr operator rgb() const;
};

struct hsva
{
	using type = float;

	_FISSION_IMPLEMENT_BASIC_COLORA(hsva, h, s, v, a);
	
	constexpr hsva(
		hsv  const& _Color,
		type const& _Alpha = impl::max_color_value<type>
	):
		h(_Color.h), s(_Color.s), v(_Color.v), a(_Alpha)
	{}

	constexpr operator rgba() const;
};

struct hsl
{
	using type = float;

	_FISSION_IMPLEMENT_BASIC_COLOR(hsl, h, s, l);

	constexpr operator rgb() const;
};

struct hsla
{
	using type = float;

	_FISSION_IMPLEMENT_BASIC_COLORA(hsla, h, s, l, a);
	
	constexpr hsla(
		hsl  const& _Color,
		type const& _Alpha = impl::max_color_value<type>
	):
		h(_Color.h), s(_Color.s), l(_Color.l), a(_Alpha)
	{}
	
	constexpr operator rgba() const;
};

struct lab
{
	using type = float;

	_FISSION_IMPLEMENT_BASIC_COLOR(lab, l, a, b);
};

struct laba
{
	using type = float;

	_FISSION_IMPLEMENT_BASIC_COLORA(laba, l, a, b, alpha);
	
	constexpr laba(
		lab  const& _Color,
		type const& _Alpha = impl::max_color_value<type>
	):
		l(_Color.l), a(_Color.a), b(_Color.b), alpha(_Alpha)
	{}
};

struct cmyk
{
	using type = float;

	type c = 0;
	type m = 0;
	type y = 0;
	type k = 0;

	constexpr bool operator==(cmyk const& _Right) const {
		return(c==_Right.c)&&(m==_Right.m)&&(y==_Right.y)&&(k==_Right.k);
	}
};



///////////////////////////////////////////////////////////////////////////////
// [Color Implementation]
///////////////////////////////////////////////////////////////////////////////

// *** (Color Conversions) ***

inline constexpr rgb::operator hsv() const
{
	struct hsv out {};
	float x_max {}, x_min {}, c {};

	if( r > g ) // R > G
	{
		x_min = g < b ? g : b;
		if( r > b ) {
			x_max = r;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : (g - b) / ( 6.0f * c );
		} else {
			x_max = b;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : type(2.0/3.0) + (r - g) / (6.0f * c);
		}
	}
	else // G > R
	{
		x_min = r < b ? r : b;
		if( g > b ) {
			x_max = g;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : type(1.0/3.0) + (b - r) / (6.0f * c);
		} else {
			x_max = b;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : type(2.0/3.0) + (r - g) / (6.0f * c);
		}
	}

	out.h = out.h >= 0? out.h:out.h + 1.0f;
	out.v = x_max;
	out.s = out.v == 0.0f ? 0.0f : c / out.v;

	return out;
}

inline constexpr rgb::operator hsl() const
{
	struct hsl out {};
	float x_max {}, x_min {}, c {};

	if( r > g ) // R > G
	{
		x_min = g < b ? g : b;
		if( r > b ) {
			x_max = r;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : (g - b) / (6.0f * c);
		}
		else {
			x_max = b;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : type(2.0/3.0) + (r - g) / (6.0f * c);
		}
	}
	else // G > R
	{
		x_min = r < b ? r : b;
		if( g > b ) {
			x_max = g;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : type(1.0/3.0) + (b - r) / (6.0f * c);
		}
		else {
			x_max = b;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : type(2.0/3.0) + (r - g) / (6.0f * c);
		}
	}

	out.h = out.h >= 0 ? out.h : out.h + 1.0f;
	out.l = 0.5f * (x_max + x_min);
	out.s = (out.l == 0.0f || out.l == 1.0f)?
		0.0f : (x_max - out.l) / math::min(out.l, 1.0f - out.l);

	return out;
}

inline constexpr hsv::operator rgb() const
{
	using math::min;
	using math::max;
	struct rgb out {};

	auto f = [&] (float n) constexpr {
		auto k = experimental::fp_mod(n + h * 6.0f, 6.0f);
		return v * (1.0f - s * max(0.0f, min(min(k, 4.0f - k), 1.0f)));
	};

	out.r = f(5.0f);
	out.g = f(3.0f);
	out.b = f(1.0f);

	return out;
}

inline constexpr hsl::operator rgb() const
{
	using math::min;
	using math::max;
	struct rgb out {};

	auto f = [&] (float n) constexpr {
		const type k = experimental::fp_mod(n + h * 12.0f, 12.0f);
		const type f1 = min(l, 1.0f - l);
		const type f2 = max(-1.0f, min(min(k - 3.0f, 9.0f - k), 1.0f));
		return l - s * f1 * f2;
	};

	out.r = f(0.0f);
	out.g = f(8.0f);
	out.b = f(4.0f);

	return out;
}


// *** (Trivial Conversions) ***

inline constexpr rgba::operator hsva() const
{
	return hsva{ static_cast<hsv>(rgb{r, g, b}), a };
}
inline constexpr rgba::operator hsla() const
{
	return hsla{ static_cast<hsl>(rgb{r, g, b}), a };
}

inline constexpr hsva::operator rgba() const
{
	return rgba{ static_cast<rgb>(hsv{h, s, v}), a };
}
inline constexpr hsla::operator rgba() const
{
	return rgba{ static_cast<rgb>(hsl{h, s, l}), a };
}

inline constexpr rgb8::operator rgb() const
{
	return rgb {
		static_cast<rgb::type>(r) / 0xFFp0f,
		static_cast<rgb::type>(g) / 0xFFp0f,
		static_cast<rgb::type>(b) / 0xFFp0f
	};
}
inline constexpr rgb::operator rgb8() const
{
	return rgb8 {
		static_cast<rgb8::type>(r * 0xFFp0f),
		static_cast<rgb8::type>(g * 0xFFp0f),
		static_cast<rgb8::type>(b * 0xFFp0f)
	};
}

inline constexpr rgba8::operator rgba() const
{
	return rgba {
		static_cast<rgba::type>(r) / 0xFFp0f,
		static_cast<rgba::type>(g) / 0xFFp0f,
		static_cast<rgba::type>(b) / 0xFFp0f,
		static_cast<rgba::type>(a) / 0xFFp0f
	};
}
inline constexpr rgba::operator rgba8() const
{
	return rgba8 {
		static_cast<rgba8::type>(r * 0xFFp0f),
		static_cast<rgba8::type>(g * 0xFFp0f),
		static_cast<rgba8::type>(b * 0xFFp0f),
		static_cast<rgba8::type>(a * 0xFFp0f)
	};
}

namespace colors
{
	static constexpr rgb  gray(float _Light) {
		return { _Light, _Light, _Light };
	};
	static constexpr rgba gray(float _Light, float _Alpha) {
		return { _Light, _Light, _Light, _Alpha };
	};
}

__FISSION_END__

#undef _FISSION_IMPLEMENT_BASIC_COLOR
#undef _FISSION_IMPLEMENT_BASIC_COLORA

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
