/**
 * @file Color.hpp
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
 * 
 * 
 * 	INDEX:
 *
 * [Color Types]
 * [Known Colors]
 * [Color Definitions]
 * [Color Implementation]
 * 	(Color Conversions)
 * 	(Trivial Conversions)
 */
#pragma once
#include <Fission/Base/Types.hpp>
#include <Fission/Base/Math/Library.hpp> // math constants & fp_mod
#include <Fission/Base/util/Operators.hpp> // math operators for `rgb` & `rgba`

#define _FISSION_DEFINE_BASIC_COLOR(TYPE, X1, X2, X3) \
type X1, X2, X3; \
constexpr TYPE()noexcept:X1(static_cast<type>(0)),X2(static_cast<type>(0)),X3(static_cast<type>(0)){} \
constexpr TYPE(type const& X1,type const& X2,type const& X3)noexcept:X1(X1),X2(X2),X3(X3){} \
constexpr bool operator==(TYPE const&)const=default

#define _FISSION_DEFINE_BASIC_COLORA(TYPE, X1, X2, X3, X4) \
type X1, X2, X3, X4; \
constexpr TYPE()noexcept:X1(static_cast<type>(0)),X2(static_cast<type>(0)),X3(static_cast<type>(0)),X4(static_cast<type>(0)){} \
constexpr TYPE(type const& X1,type const& X2,type const& X3,type const& X4 = impl::max_color_value<type>)noexcept:X1(X1),X2(X2),X3(X3),X4(X4){} \
constexpr bool operator==(TYPE const&)const=default

__FISSION_BEGIN__


/* ======================================== [Color Types] ======================================== */

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


/* ======================================= [Known Colors] ======================================== */
namespace colors
{
	enum known : color_t
	{
		// ColorName = 0xRRGGBBAA
		
		// Pink Colors

		MediumVioletRed = 0xC71585FF,
		DeepPink		= 0xFF1493FF,
		PaleVioletRed	= 0xDB7093FF,
		HotPink			= 0xFF69B4FF,
		LightPink		= 0xFFB6C1FF,
		Pink			= 0xFFC0CBFF,

		// Red Colors

		DarkRed		= 0x8B0000FF,
		Red			= 0xFF0000FF,
		Firebrick	= 0xB22222FF,
		Crimson		= 0xDC143CFF,
		IndianRed	= 0xCD5C5CFF,
		LightCoral	= 0xF08080FF,
		Salmon		= 0xFA8072FF,
		DarkSalmon	= 0xE9967AFF,
		LightSalmon = 0xFFA07AFF,

		// Orange Colors

		OrangeRed	= 0x4500FFFF,
		Tomato		= 0x6347FFFF,
		DarkOrange	= 0x8C00FFFF,
		Coral		= 0x7F50FFFF,
		Orange		= 0xA500FFFF,

		// Yellow Colors

		DarkKhaki				= 0xBDB76BFF,
		Gold					= 0xFFD700FF,
		Khaki					= 0xF0E68CFF,
		PeachPuff				= 0xFFDAB9FF,
		Yellow					= 0xFFFF00FF,
		PaleGoldenrod			= 0xEEE8AAFF,
		Moccasin				= 0xFFE4B5FF,
		PapayaWhip				= 0xFFEFD5FF,
		LightGoldenrodYellow	= 0xFAFAD2FF,
		LemonChiffon			= 0xFFFACDFF,
		LightYellow				= 0xFFFFE0FF,

		// Brown Colors

		Maroon 				= 0x800000FF,
		Brown 				= 0xA52A2AFF,
		SaddleBrown 		= 0x8B4513FF,
		Sienna 				= 0xA0522DFF,
		Chocolate 			= 0xD2691EFF,
		DarkGoldenrod 		= 0xB8860BFF,
		Peru 				= 0xCD853FFF,
		RosyBrown 			= 0xBC8F8FFF,
		Goldenrod 			= 0xDAA520FF,
		SandyBrown			= 0xF4A460FF,
		Tan 				= 0xD2B48CFF,
		Burlywood 			= 0xDEB887FF,
		Wheat 				= 0xF5DEB3FF,
		NavajoWhite 		= 0xFFDEADFF,
		Bisque 				= 0xFFE4C4FF,
		BlanchedAlmond 		= 0xFFEBCDFF,
		Cornsilk 			= 0xFFF8DCFF,

		// Green Colors

		DarkGreen 			= 0x006400FF,
		Green 				= 0x008000FF,
		DarkOliveGreen 		= 0x556B2FFF,
		ForestGreen 		= 0x228B22FF,
		SeaGreen 			= 0x2E8B57FF,
		Olive 				= 0x808000FF,
		OliveDrab 			= 0x6B8E23FF,
		MediumSeaGreen 		= 0x3CB371FF,
		LimeGreen 			= 0x32CD32FF,
		Lime 				= 0x00FF00FF,
		SpringGreen 		= 0x00FF7FFF,
		MediumSpringGreen 	= 0x00FA9AFF,
		DarkSeaGreen 		= 0x8FBC8FFF,
		MediumAquamarine 	= 0x66CDAAFF,
		YellowGreen 		= 0x9ACD32FF,
		LawnGreen 			= 0x7CFC00FF,
		Chartreuse 			= 0x7FFF00FF,
		LightGreen 			= 0x90EE90FF,
		GreenYellow 		= 0xADFF2FFF,
		PaleGreen 			= 0x98FB98FF,

		// Cyan Colors

		Teal 			= 0x008080FF,
		DarkCyan 		= 0x008B8BFF,
		LightSeaGreen 	= 0x20B2AAFF,
		CadetBlue 		= 0x5F9EA0FF,
		DarkTurquoise	= 0x00CED1FF,
		MediumTurquoise	= 0x48D1CCFF,
		Turquoise		= 0x40E0D0FF,
		Aqua			= 0x00FFFFFF,
		Cyan			= 0x00FFFFFF,
		Aquamarine		= 0x7FFFD4FF,
		PaleTurquoise	= 0xAFEEEEFF,
		LightCyan		= 0xE0FFFFFF,

		// Blue Colors

		Navy			= 0x000080FF,
		DarkBlue		= 0x00008BFF,
		MediumBlue		= 0x0000CDFF,
		Blue			= 0x0000FFFF,
		MidnightBlue	= 0x191970FF,
		RoyalBlue		= 0x4169E1FF,
		SteelBlue		= 0x4682B4FF,
		DodgerBlue		= 0x1E90FFFF,
		DeepSkyBlue		= 0x00BFFFFF,
		CornflowerBlue	= 0x6495EDFF,
		SkyBlue			= 0x87CEEBFF,
		LightSkyBlue	= 0x87CEFAFF,
		LightSteelBlue	= 0xB0C4DEFF,
		LightBlue		= 0xADD8E6FF,
		PowderBlue		= 0xB0E0E6FF,

		// Purple, Violet, and Magenta Colors

		Indigo			= 0x4B0082FF,
		Purple			= 0x800080FF,
		DarkMagenta		= 0x8B008BFF,
		DarkViolet		= 0x9400D3FF,
		DarkSlateBlue	= 0x483D8BFF,
		BlueViolet		= 0x8A2BE2FF,
		DarkOrchid		= 0x9932CCFF,
		Fuchsia			= 0xFF00FFFF,
		Magenta			= 0xFF00FFFF,
		SlateBlue		= 0x6A5ACDFF,
		MediumSlateBlue	= 0x7B68EEFF,
		MediumOrchid	= 0xBA55D3FF,
		MediumPurple	= 0x9370DBFF,
		Orchid			= 0xDA70D6FF,
		Violet			= 0xEE82EEFF,
		Plum			= 0xDDA0DDFF,
		Thistle			= 0xD8BFD8FF,
		Lavender		= 0xE6E6FAFF,

		// White Colors

		MistyRose		= 0xFFE4E1FF,
		AntiqueWhite	= 0xFAEBD7FF,
		Linen			= 0xFAF0E6FF,
		Beige			= 0xF5F5DCFF,
		WhiteSmoke		= 0xF5F5F5FF,
		LavenderBlush	= 0xFFF0F5FF,
		OldLace			= 0xFDF5E6FF,
		AliceBlue		= 0xF0F8FFFF,
		Seashell		= 0xFFF5EEFF,
		GhostWhite		= 0xF8F8FFFF,
		Honeydew		= 0xF0FFF0FF,
		FloralWhite		= 0xFFFAF0FF,
		Azure			= 0xF0FFFFFF,
		MintCream		= 0xF5FFFAFF,
		Snow			= 0xFFFAFAFF,
		Ivory			= 0xFFFFF0FF,
		White			= 0xFFFFFFFF,

		// Gray and Black Colors

		Black			= 0x000000FF,
		DarkSlateGray	= 0x2F4F4FFF,
		DimGray			= 0x696969FF,
		SlateGray		= 0x708090FF,
		Gray			= 0x808080FF,
		LightSlateGray	= 0x778899FF,
		DarkGray		= 0xA9A9A9FF,
		Silver			= 0xC0C0C0FF,
		LightGray		= 0xD3D3D3FF,
		Gainsboro		= 0xDCDCDCFF,

	}; // enum known
	
	template <color_t HEX>
	static constexpr known make = static_cast<known>( (HEX << 8) | 0xFF );

} // namespace Fission::colors

namespace impl
{
	template <typename T> requires std::is_arithmetic_v<T>
	static constexpr T max_color_value = std::is_integral_v<T> ?
		std::numeric_limits<T>::max() : static_cast<T>( 1 );
}


/* ===================================== [Color Definitions] ===================================== */

struct rgb8
{
	using type = u8;

	_FISSION_DEFINE_BASIC_COLOR( rgb8, r, g, b );

	constexpr rgb8(colors::known const& c)noexcept:
		r( static_cast<type>(c >> 24) ),
		g( static_cast<type>(c >> 16) ),
		b( static_cast<type>(c >> 8) )
	{}

	constexpr operator rgb() const noexcept;
};

struct rgba8
{
	using type = u8;

	_FISSION_DEFINE_BASIC_COLORA( rgba8, r, g, b, a );
	
	constexpr rgba8( rgb8 const& _Color, type const& _Alpha = impl::max_color_value<type> )noexcept:
		r(_Color.r), g(_Color.g), b(_Color.b), a(_Alpha)
	{}

	constexpr rgba8( colors::known const& c )noexcept:
		r( static_cast<type>( c >> 24 ) ),
		g( static_cast<type>( c >> 16 ) ),
		b( static_cast<type>( c >> 8 ) ),
		a( static_cast<type>( c ) )
	{}

	constexpr operator rgba() const noexcept;
};

struct rgb
{
	using type = float;

	_FISSION_DEFINE_BASIC_COLOR( rgb, r, g, b );
	
	constexpr rgb(colors::known const& c)noexcept:
		r( static_cast<type>((c >> 24) & 0xFF) / 0xFFp0f ),
		g( static_cast<type>((c >> 16) & 0xFF) / 0xFFp0f ),
		b( static_cast<type>((c >> 8)  & 0xFF) / 0xFFp0f )
	{}

	constexpr operator rgb8() const noexcept;

	constexpr operator hsv() const noexcept;
	constexpr operator hsl() const noexcept;

	_FISSION_DEFINE_OPERATORS_3(rgb, type, r, g, b);
};
_FISSION_DEFINE_OPERATOR_MULTIPLY_3(rgb, rgb::type, r, g, b)

struct rgba
{
	using type = float;

	_FISSION_DEFINE_BASIC_COLORA( rgba, r, g, b, a );
	
	constexpr rgba( rgb const& _Color, type const& _Alpha = impl::max_color_value<type> )noexcept:
		r(_Color.r), g(_Color.g), b(_Color.b), a(_Alpha)
	{}
	
	constexpr rgba(colors::known const& c)noexcept:
		r( static_cast<type>((c >> 24) & 0xFF) / 0xFFp0f ),
		g( static_cast<type>((c >> 16) & 0xFF) / 0xFFp0f ),
		b( static_cast<type>((c >> 8)  & 0xFF) / 0xFFp0f ),
		a( static_cast<type>((c)       & 0xFF) / 0xFFp0f )
	{}

	constexpr operator rgba8() const noexcept;

	constexpr operator hsva() const noexcept;
	constexpr operator hsla() const noexcept;

	_FISSION_DEFINE_OPERATORS_4(rgba, type, r, g, b, a);
};
_FISSION_DEFINE_OPERATOR_MULTIPLY_4(rgba, rgba::type, r, g, b, a)

struct hsv
{
	using type = float;

	_FISSION_DEFINE_BASIC_COLOR( hsv, h, s, v );

	constexpr operator rgb() const noexcept;
};

struct hsva
{
	using type = float;

	_FISSION_DEFINE_BASIC_COLORA( hsva, h, s, v, a );
	
	constexpr hsva( hsv const& _Color, type const& _Alpha = impl::max_color_value<type> )noexcept:
		h(_Color.h), s(_Color.s), v(_Color.v), a(_Alpha)
	{}

	constexpr operator rgba() const noexcept;
};

struct hsl
{
	using type = float;

	_FISSION_DEFINE_BASIC_COLOR( hsl, h, s, l );

	constexpr operator rgb() const noexcept;
};

struct hsla
{
	using type = float;

	_FISSION_DEFINE_BASIC_COLORA( hsla, h, s, l, a );
	
	constexpr hsla( hsl const& _Color, type const& _Alpha = impl::max_color_value<type> )noexcept:
		h(_Color.h), s(_Color.s), l(_Color.l), a(_Alpha)
	{}
	
	constexpr operator rgba() const noexcept;
};

struct lab
{
	using type = float;

	_FISSION_DEFINE_BASIC_COLOR( lab, l, a, b );
};

struct laba
{
	using type = float;

	_FISSION_DEFINE_BASIC_COLORA( laba, l, a, b, alpha );
	
	constexpr laba( lab const& _Color, type const& _Alpha = impl::max_color_value<type> )noexcept:
		l(_Color.l), a(_Color.a), b(_Color.b), alpha(_Alpha)
	{}
};

struct cmyk
{
	using type = float;

	type c, m, y, k;

	constexpr cmyk()noexcept:c(static_cast<type>(0)),m(static_cast<type>(0)),y(static_cast<type>(0)),k(static_cast<type>(0)){}
	constexpr cmyk(type const& c, type const& m, type const& y, type const& k)noexcept:c(c),m(m),y(y),k(k){}
	constexpr bool operator==( cmyk const& )const = default;
};


/* =================================== [Color Implementation] ==================================== */

/* ===================================== (Color Conversions) ===================================== */

inline constexpr rgb::operator hsv() const noexcept
{
	struct hsv out {};
	float x_max, x_min, c;

	if( r > g ) // R > G
	{
		x_min = g < b ? g : b;
		if( r > b ) {
			x_max = r;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : ( g - b ) / ( 6.0f * c );
		} else {
			x_max = b;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : math::two_thirds<type> + ( r - g ) / ( 6.0f * c );
		}
	}
	else // G > R
	{
		x_min = r < b ? r : b;
		if( g > b ) {
			x_max = g;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : math::one_third<type> + ( b - r ) / ( 6.0f * c );
		} else {
			x_max = b;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : math::two_thirds<type> + ( r - g ) / ( 6.0f * c );
		}
	}

	out.h = out.h >= 0? out.h:out.h + 1.0f;
	out.v = x_max;
	out.s = out.v == 0.0f ? 0.0f : c / out.v;

	return out;
}

inline constexpr rgb::operator hsl() const noexcept
{
	struct hsl out {};
	float x_max, x_min, c;

	if( r > g ) // R > G
	{
		x_min = g < b ? g : b;
		if( r > b ) {
			x_max = r;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : ( g - b ) / ( 6.0f * c );
		}
		else {
			x_max = b;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : math::two_thirds<type> +( r - g ) / ( 6.0f * c );
		}
	}
	else // G > R
	{
		x_min = r < b ? r : b;
		if( g > b ) {
			x_max = g;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : math::one_third<type> +( b - r ) / ( 6.0f * c );
		}
		else {
			x_max = b;
			c = x_max - x_min;
			out.h = c == 0.0f ? 0.0f : math::two_thirds<type> +( r - g ) / ( 6.0f * c );
		}
	}

	out.h = out.h >= 0 ? out.h : out.h + 1.0f;
	out.l = 0.5f * (x_max + x_min);
	out.s = (out.l == 0.0f || out.l == 1.0f) ? 0.0f : (x_max - out.l) / std::min(out.l, 1.0f - out.l);

	return out;
}

inline constexpr hsv::operator rgb() const noexcept
{
	struct rgb out {};

	auto f = [&]( float n )
	{
		auto k = experimental::fp_mod( n + h * 6.0f, 6.0f );
		return v * ( 1.0f - s * std::max( 0.0f, std::min( std::min( k, 4.0f - k ), 1.0f ) ) );
	};

	out.r = f( 5.0f );
	out.g = f( 3.0f );
	out.b = f( 1.0f );

	return out;
}

inline constexpr hsl::operator rgb() const noexcept
{
	struct rgb out {};

	auto f = [&]( float n )
	{
		const type k = experimental::fp_mod( n + h * 12.0f, 12.0f );
		return l - s * std::min( l, 1.0f - l ) * std::max(-1.0f, std::min(std::min(k - 3.0f, 9.0f - k), 1.0f));
	};

	out.r = f( 0.0f );
	out.g = f( 8.0f );
	out.b = f( 4.0f );

	return out;
}


/* ==================================== (Trivial Conversions) ==================================== */


inline constexpr rgba::operator hsva() const noexcept
{
	return hsva{ static_cast<hsv>( rgb{r, g, b} ), a };
}
inline constexpr rgba::operator hsla() const noexcept
{
	return hsla{ static_cast<hsl>( rgb{r, g, b} ), a };
}

inline constexpr hsva::operator rgba() const noexcept
{
	return rgba{ static_cast<rgb>( hsv{h, s, v} ), a };
}
inline constexpr hsla::operator rgba() const noexcept
{
	return rgba{ static_cast<rgb>( hsl{h, s, l} ), a };
}

inline constexpr rgb8::operator rgb() const noexcept
{
	return rgb {
		static_cast<rgb::type>(r) / 0xFFp0f,
		static_cast<rgb::type>(g) / 0xFFp0f,
		static_cast<rgb::type>(b) / 0xFFp0f
	};
}
inline constexpr rgb::operator rgb8() const noexcept
{
	return rgb8 {
		static_cast<rgb8::type>(r * 0xFFp0f),
		static_cast<rgb8::type>(g * 0xFFp0f),
		static_cast<rgb8::type>(b * 0xFFp0f)
	};
}

inline constexpr rgba8::operator rgba() const noexcept
{
	return rgba {
		static_cast<rgba::type>(r) / 0xFFp0f,
		static_cast<rgba::type>(g) / 0xFFp0f,
		static_cast<rgba::type>(b) / 0xFFp0f,
		static_cast<rgba::type>(a) / 0xFFp0f
	};
}
inline constexpr rgba::operator rgba8() const noexcept
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
	static constexpr rgb  gray( float _Light ) { return{ _Light, _Light, _Light }; };
	static constexpr rgba gray( float _Light, float _Alpha ) { return{ _Light, _Light, _Light, _Alpha }; };
}

__FISSION_END__
