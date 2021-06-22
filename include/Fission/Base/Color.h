/**
*
* @file: Color.h
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

/**
* 	INDEX:
* 	
* [Color Types]
* [Known Colors]
* [Color Definitions]
* [Color Helpers]
* [Color Implementation]
* 	(Color Arithmetic)
* 	(Color Conversions)
*/

_FISSION_BASE_PUBLIC_BEGIN

/* ======================== [Color Types] ======================== */

/* Opaque Colors which use single-persision floating point */
struct rgb_colorf;
struct hsv_colorf;

/* Opaque Colors which use 8-bit integers */
struct rgb_color8;
struct hsv_color8;
	
/* Colors which use single-persision floating point */
struct rgba_colorf;
struct hsva_colorf;

/* Colors which use 8-bit integers */
struct rgba_color8;
struct hsva_color8;


/* Select default structure to use for color */
using color = rgba_colorf;



/* ======================== [Known Colors] ======================== */

namespace Colors {
	/* Format: 0xRRGGBB */
	enum KnownColor : unsigned int {
		Red              = 0xff0000,
		DarkRed          = 0x8b0000,
		FireBrick        = 0xb22222,
		Crimson          = 0xdc143c,
		IndianRed        = 0xcd5c5c,
		LightCoral       = 0xf08080,
		DarkSalmon       = 0xe9967a,
		Salmon           = 0xfa8072,
		LightSalmon      = 0xffa07a,
		Tomato           = 0xff6347,
		Coral            = 0xff7f50,
		OrangeRed        = 0xff4500,
		Orange           = 0xffa500,
		DarkOrange       = 0xff8c00,
		Gold             = 0xffd700,
		Yellow           = 0xffff00,
		DarkKhaki        = 0xbdb76b,
		Khaki            = 0xf0e68c,
		PaleGoldenrod    = 0xeee8aa,
		PeachPuff        = 0xffdab9,
		Moccasin         = 0xffe4b5,
		PapayaWhip       = 0xffefd5,
		LightGoldYellow  = 0xfafad2,
		LemonChiffon     = 0xfffacd,
		LightYellow      = 0xffffe0,
		LawnGreen        = 0x7cfc00,
		Chartreuse       = 0x7fff00,
		LimeGreen        = 0x32cd32,
		Lime             = 0x00ff00,
		ForestGreen      = 0x228b22,
		Green            = 0x008000,
		DarkGreen        = 0x006400,
		GreenYellow      = 0xadff2f,
		LightGreen       = 0x90ee90,
		MedSpringGreen   = 0x00fa9a,
		SpringGreen      = 0x00ff7f,
		YellowGreen      = 0x9acd32,
		PaleGreen        = 0x98fb98,
		DarkSeaGreen     = 0x8fbc8f,
		MediumSeaGreen   = 0x3cb371,
		SeaGreen         = 0x2e8b57,
		Olive            = 0x808000,
		DarkOliveGreen   = 0x556b2f,
		OliveDrab        = 0x6b8e23,
		LightCyan        = 0xe0ffff,
		Cyan             = 0x00ffff,
		Aqua             = 0x00ffff,
		Aquamarine       = 0x7fffd4,
		MedAquamarine    = 0x66cdaa,
		PaleTurquoise    = 0xafeeee,
		Turquoise        = 0x40e0d0,
		MedTurquoise     = 0x48d1cc,
		DarkTurquiose    = 0x00ced1,
		LightSeaGreen    = 0x20b2aa,
		CadetBlue        = 0x5f9ea0,
		DarkCyan         = 0x008b8b,
		Teal             = 0x008080,
		PowderBlue       = 0xb0e0e6,
		LightBlue        = 0xadd8e6,
		LightSkyBlue     = 0x87cefa,
		SkyBlue          = 0x87ceeb,
		DeepSkyBlue      = 0x00bfff,
		LightSteelBlue   = 0xb0c4de,
		DodgerBlue       = 0x1e90ff,
		CornflowerBlue   = 0x6495ed,
		SteelBlue        = 0x4682b4,
		RoyalBlue        = 0x4169e1,
		Blue             = 0x0000ff,
		MediumBlue       = 0x0000cd,
		DarkBlue         = 0x00008b,
		Navy             = 0x000080,
		MidnightBlue     = 0x191970,
		MediumSlateBlue  = 0x7b68ee,
		SlateBlue        = 0x6a5acd,
		DarkSlateBlue    = 0x483d8b,
		Lavender         = 0xe6e6fa,
		Thristle         = 0xd8bfd8,
		Plum             = 0xdda0dd,
		Violet           = 0xee82ee,
		Orchid           = 0xda70d6,
		Magenta          = 0xff00ff,
		MediumOrchid     = 0xba55d3,
		MediumPurple     = 0x9370db,
		BlueViolet       = 0x8a2be2,
		DarkViolet       = 0x9400d3,
		DarkOrchid       = 0x9932cc,
		DarkMagenta      = 0x8b008b,
		Purple           = 0x800080,
		Indigo           = 0x4b0082,
		Pink             = 0xffc0cb,
		LightPink        = 0xffb6c1,
		HotPink          = 0xff69b4,
		DeepPink         = 0xff1493,
		PaleVioletRed    = 0xdb7093,
		MediumVioletRed  = 0xc71585,
		White            = 0xffffff,
		Snow             = 0xfffafa,
		Honeydew         = 0xf0fff0,
		MintCream        = 0xf5fffa,
		Azure            = 0xf0ffff,
		AliceBlue        = 0xf0f8ff,
		GhostWhite       = 0xf8f8ff,
		WhiteSmoke       = 0xf5f5f5,
		Seashell         = 0xfff5ee,
		Beige            = 0xf5f5dc,
		OldLace          = 0xfdf5e6,
		FloralWhite      = 0xfffaf0,
		Ivory            = 0xfffff0,
		AntiqueWhite     = 0xfaebd7,
		Linen            = 0xfaf0e6,
		LavenderBlush    = 0xfff0f5,
		MistyRose        = 0xffe4e1,
		Gainsboro        = 0xdcdcdc,
		LightGray        = 0xd3d3d3,
		Silver           = 0xc0c0c0,
		DarkGray         = 0xa9a9a9,
		Gray             = 0x808080,
		DimGray          = 0x696969,
		LightSlateGray   = 0x778899,
		SlateGray        = 0x708090,
		DarkSlateGray    = 0x2f4f4f,
		Black            = 0x000000,
		Cornsilk         = 0xfff8dc,
		Blanchedalmond   = 0xffebcd,
		Bisque           = 0xffe4c4,
		NavajoWhite      = 0xffdead,
		Wheat            = 0xf5deb3,
		Burlywood        = 0xdeb887,
		Tan              = 0xd2b48c,
		RosyBrown        = 0xbc8f8f,
		SandyBrown       = 0xf4a460,
		Goldenrod        = 0xdaa520,
		Peru             = 0xcd853f,
		Chocolate        = 0xd2691e,
		SaddleBrown      = 0x8b4513,
		Sienna           = 0xa0522d,
		Brown            = 0xa52a2a,
		Maroon           = 0x800000,
	}; // enum Fission::Colors::KnownColor

} // namespace Fission::Colors



/* ======================== [Color Definitions] ======================== */

//! @struct rgb_colorf
struct rgb_colorf {
	float r, g, b;

	using value_type = float;

	constexpr rgb_colorf();
	constexpr rgb_colorf( float _R, float _G, float _B );
	constexpr rgb_colorf( Colors::KnownColor _Known_Color );

	constexpr rgb_colorf( const hsv_colorf & _Source );
	constexpr rgb_colorf( const rgb_color8 & _Source );

	explicit constexpr rgb_colorf( const rgba_colorf & _Source );
	explicit constexpr rgb_colorf( const hsva_colorf & _Source );
	explicit constexpr rgb_colorf( const rgba_color8 & _Source );
		
}; // struct Fission::rgb_colorf

//! @struct hsv_colorf
struct hsv_colorf {
	float h, s, v;

	using value_type = float;

	constexpr hsv_colorf();
	constexpr hsv_colorf( float _H, float _S, float _V );
	constexpr hsv_colorf( Colors::KnownColor _Known_Color );

	constexpr hsv_colorf( const rgb_colorf & _Source );
	constexpr hsv_colorf( const hsv_color8 & _Source );

	explicit constexpr hsv_colorf( const rgba_colorf & _Source );
	explicit constexpr hsv_colorf( const hsva_colorf & _Source );
	explicit constexpr hsv_colorf( const hsva_color8 & _Source );

}; // struct Fission::hsv_colorf

//! @struct rgb_color8
struct rgb_color8 {
	unsigned char r, g, b;

	using value_type = unsigned char;

	constexpr rgb_color8();
	constexpr rgb_color8( unsigned char _R, unsigned char _G, unsigned char _B );
	constexpr rgb_color8( Colors::KnownColor _Known_Color );

	explicit constexpr rgb_color8( const rgb_colorf & _Source );
	explicit constexpr rgb_color8( const rgba_colorf & _Source );
	explicit constexpr rgb_color8( const rgba_color8 & _Source );

}; // struct Fission::rgb_color8

//! @struct hsv_color8
struct hsv_color8 {
	unsigned char h, s, v;

	using value_type = unsigned char;

	constexpr hsv_color8();
	constexpr hsv_color8( unsigned char _H, unsigned char _S, unsigned char _V );
	constexpr hsv_color8( Colors::KnownColor _Known_Color );

	explicit constexpr hsv_color8( const hsv_colorf & _Source );
	explicit constexpr hsv_color8( const hsva_colorf & _Source );
	explicit constexpr hsv_color8( const hsva_color8 & _Source );

}; // struct Fission::hsv_color8

//! @struct rgba_colorf
struct rgba_colorf {
	float r, g, b, a;

	using value_type = float;

	constexpr rgba_colorf( float _Alpha = 1.0f );
	constexpr rgba_colorf( float _R, float _G, float _B, float _A = 1.0f );
	constexpr rgba_colorf( Colors::KnownColor _Known_Color, float _Alpha = 1.0f );

	constexpr rgba_colorf( const rgba_colorf & _Source, float _Alpha );

	constexpr rgba_colorf( const rgb_colorf & _Source, float _Alpha = 1.0f );
	constexpr rgba_colorf( const hsv_colorf & _Source, float _Alpha = 1.0f );
	constexpr rgba_colorf( const hsva_colorf & _Source );

	constexpr rgba_colorf( const rgb_color8 & _Source );
	constexpr rgba_colorf( const hsv_color8 & _Source );
	constexpr rgba_colorf( const rgba_color8 & _Source );
	constexpr rgba_colorf( const hsva_color8 & _Source );


	constexpr rgba_colorf operator+( const rgba_colorf& rhs ) const;
	constexpr rgba_colorf operator-( const rgba_colorf& rhs ) const;
	constexpr rgba_colorf operator*( const rgba_colorf& rhs ) const;
	constexpr rgba_colorf operator/( const rgba_colorf& rhs ) const;
	constexpr rgba_colorf operator*( const float& rhs ) const;
	constexpr rgba_colorf operator/( const float& rhs ) const;

	rgba_colorf & operator+=( const rgba_colorf& rhs );
	rgba_colorf & operator-=( const rgba_colorf& rhs );
	rgba_colorf & operator*=( const rgba_colorf& rhs );
	rgba_colorf & operator/=( const rgba_colorf& rhs );
	rgba_colorf & operator*=( const float& rhs );
	rgba_colorf & operator/=( const float& rhs );

}; // struct Fission::rgba_colorf

//! @struct hsva_colorf
struct hsva_colorf {
	float h, s, v, a;

	using value_type = float;

	constexpr hsva_colorf( float _Alpha = 1.0f );
	constexpr hsva_colorf( float _H, float _S, float _V, float _A = 1.0f );
	constexpr hsva_colorf( Colors::KnownColor _Known_Color, float _Alpha = 1.0f );

	explicit constexpr hsva_colorf( const rgb_colorf & _Source, float _Alpha = 1.0f );
	explicit constexpr hsva_colorf( const rgb_color8 & _Source );
	explicit constexpr hsva_colorf( const hsv_colorf & _Source );
	explicit constexpr hsva_colorf( const hsv_color8 & _Source );

	constexpr hsva_colorf( const rgba_colorf & _Source );
	constexpr hsva_colorf( const rgba_color8 & _Source );
	constexpr hsva_colorf( const hsva_color8 & _Source );

}; // struct Fission::hsva_colorf

//! @struct rgba_color8
struct rgba_color8 {
	unsigned char r, g, b, a;

	using value_type = unsigned char;

	constexpr rgba_color8( unsigned char _Alpha = 1.0f );
	constexpr rgba_color8( unsigned char _R, unsigned char _G, unsigned char _B, unsigned char _A = 1.0f );
	constexpr rgba_color8( Colors::KnownColor _Known_Color, unsigned char _Alpha = 255 );
		
	constexpr rgba_color8( const rgb_colorf & _Source );
	constexpr rgba_color8( const rgb_color8 & _Source );
	constexpr rgba_color8( const rgba_colorf & _Source );

	// Packs contents of the color into a single dword.
	constexpr unsigned int pack();

}; // struct Fission::rgba_color8

//! @struct hsva_color8
struct hsva_color8 {
	unsigned char h, s, v, a;

	using value_type = unsigned char;

	constexpr hsva_color8( unsigned char _Alpha = 1.0f );
	constexpr hsva_color8( unsigned char _H, unsigned char _S, unsigned char _V, unsigned char _A = 1.0f );
	constexpr hsva_color8( Colors::KnownColor _Known_Color );

	explicit constexpr hsva_color8( const hsv_colorf & _Source );
	explicit constexpr hsva_color8( const hsv_color8 & _Source );
	explicit constexpr hsva_color8( const hsva_colorf & _Source );

}; // struct Fission::hsva_color8



/* ======================== [Color Helpers] ======================== */

namespace Colors
{
	template<typename T> struct has_alpha : std::false_type {};
	template<> struct has_alpha<rgba_colorf> : std::true_type {};
	template<> struct has_alpha<rgba_color8> : std::true_type {};
	template<> struct has_alpha<hsva_colorf> : std::true_type {};
	template<> struct has_alpha<hsva_color8> : std::true_type {};
	template<typename T> static constexpr bool has_alpha_v = has_alpha<T>::value;

	template<typename T> struct is_color : std::false_type {};
	template<> struct is_color<rgb_colorf> : std::true_type {};
	template<> struct is_color<rgb_color8> : std::true_type {};
	template<> struct is_color<hsv_colorf> : std::true_type {};
	template<> struct is_color<hsv_color8> : std::true_type {};
	template<> struct is_color<rgba_colorf> : std::true_type {};
	template<> struct is_color<rgba_color8> : std::true_type {};
	template<> struct is_color<hsva_colorf> : std::true_type {};
	template<> struct is_color<hsva_color8> : std::true_type {};
	template<typename T> static constexpr bool is_color_v = is_color<T>::value;

	template<typename T> struct is_rgb : std::false_type {};
	template<> struct is_rgb<rgb_colorf> : std::true_type {};
	template<> struct is_rgb<rgb_color8> : std::true_type {};
	template<> struct is_rgb<rgba_colorf> : std::true_type {};
	template<> struct is_rgb<rgba_color8> : std::true_type {};
	template<typename T> static constexpr bool is_rgb_v = is_rgb<T>::value;

	template<typename T> struct is_hsv : std::false_type {};
	template<> struct is_hsv<hsv_colorf> : std::true_type {};
	template<> struct is_hsv<hsv_color8> : std::true_type {};
	template<> struct is_hsv<hsva_colorf> : std::true_type {};
	template<> struct is_hsv<hsva_color8> : std::true_type {};
	template<typename T> static constexpr bool is_hsv_v = is_hsv<T>::value;


	template <typename ColorTy = rgb_colorf>
	static constexpr ColorTy make_gray( typename ColorTy::value_type _Percent_White )
	{
		static_assert( is_color_v<ColorTy> );

		if constexpr( is_rgb_v<ColorTy> )
		{
			return ColorTy( _Percent_White, _Percent_White, _Percent_White );
		}
		if constexpr( is_hsv_v<ColorTy> )
		{
			return ColorTy( 0.0f, 0.0f, _Percent_White );
		}
	}

	template <typename ColorTy = rgba_colorf>
	static constexpr ColorTy make_gray( typename ColorTy::value_type _Percent_White, typename ColorTy::value_type _Alpha )
	{
		static_assert( is_color_v<ColorTy> && has_alpha_v<ColorTy> );

		if constexpr( is_rgb_v<ColorTy> )
		{
			return ColorTy( _Percent_White, _Percent_White, _Percent_White, _Alpha );
		}
		if constexpr( is_hsv_v<ColorTy> )
		{
			return ColorTy( 0.0f, 0.0f, _Percent_White, _Alpha );
		}
	}

} // namespace Fission::Colors



/* ======================== [Color Implementation] ======================== */

/* rgb_colorf */
inline constexpr rgb_colorf::rgb_colorf() : r( 0.0f ), g( 0.0f ), b( 0.0f ) {}
inline constexpr rgb_colorf::rgb_colorf( float _R, float _G, float _B ) : r( _R ), g( _G ), b( _B ) {}
inline constexpr rgb_colorf::rgb_colorf( Colors::KnownColor _Known_Color ):
	r( float( (_Known_Color >> 16) & 0xFF ) / 255.0f ),
	g( float( (_Known_Color >> 8 ) & 0xFF ) / 255.0f ),
	b( float( (_Known_Color      ) & 0xFF ) / 255.0f )
{}
inline constexpr rgb_colorf::rgb_colorf( const rgba_colorf & _Source ) : r( _Source.r ), g( _Source.g ), b( _Source.b ) {}
inline constexpr rgb_colorf::rgb_colorf( const rgb_color8 & _Source ):
	r( float(_Source.r) / 255.0f ), g( float(_Source.g) / 255.0f ), b( float(_Source.b) / 255.0f ) {}
inline constexpr rgb_colorf::rgb_colorf( const rgba_color8 & _Source ):
	r( float(_Source.r) / 255.0f ), g( float(_Source.g) / 255.0f ), b( float(_Source.b) / 255.0f ) {}

/* hsv_colorf */
inline constexpr hsv_colorf::hsv_colorf() : h( 0.0f ), s( 0.0f ), v( 0.0f ) {}
inline constexpr hsv_colorf::hsv_colorf( float _H, float _S, float _V ) : h( _H ), s( _S ), v( _V ) {}
inline constexpr hsv_colorf::hsv_colorf( const hsva_colorf & _Source ): h(_Source.h), s(_Source.s), v(_Source.v) {}
inline constexpr hsv_colorf::hsv_colorf( const hsv_color8 & _Source ):
	h( float(_Source.h) / 255.0f ), s( float(_Source.s) / 255.0f ), v( float(_Source.v) / 255.0f ) {}
inline constexpr hsv_colorf::hsv_colorf( const hsva_color8 & _Source ):
	h( float(_Source.h) / 255.0f ), s( float(_Source.s) / 255.0f ), v( float(_Source.v) / 255.0f ) {}

/* rgb_color8 */
inline constexpr rgb_color8::rgb_color8() : r( 0 ), g( 0 ), b( 0 ) {}
inline constexpr rgb_color8::rgb_color8( unsigned char _R, unsigned char _G, unsigned char _B ) : r( _R ), g( _G ), b( _B ) {}
inline constexpr rgb_color8::rgb_color8( Colors::KnownColor _Known_Color ) :
	r( unsigned char( _Known_Color >> 16 ) ),
	g( unsigned char( _Known_Color >> 8  ) ),
	b( unsigned char( _Known_Color       ) )
{}

/* hsv_color8 */
inline constexpr hsv_color8::hsv_color8() : h( 0 ), s( 0 ), v( 0 ) {}
inline constexpr hsv_color8::hsv_color8( unsigned char _H, unsigned char _S, unsigned char _V ) : h( _H ), s( _S ), v( _V ) {}


/* rgba_colorf */
inline constexpr rgba_colorf::rgba_colorf( float _Alpha ) : r( 0.0f ), g( 0.0f ), b( 0.0f ), a( _Alpha ) {}
inline constexpr rgba_colorf::rgba_colorf( float _R, float _G, float _B, float _A ) : r( _R ), g( _G ), b( _B ), a( _A ) {}
inline constexpr rgba_colorf::rgba_colorf( Colors::KnownColor _Known_Color, float _Alpha ) :
	r( float( (_Known_Color >> 16) & 0xFF ) / 255.0f ),
	g( float( (_Known_Color >> 8 ) & 0xFF ) / 255.0f ),
	b( float( (_Known_Color      ) & 0xFF ) / 255.0f ),
	a( _Alpha )
{}
inline constexpr rgba_colorf::rgba_colorf( const rgba_color8 & _Source ): 
	r( (float)_Source.r / 255.0f ), g( (float)_Source.g / 255.0f ), b( (float)_Source.b / 255.0f ), a( (float)_Source.a / 255.0f ) {}
inline constexpr rgba_colorf::rgba_colorf( const rgb_colorf & _Source, float _Alpha ) : r( _Source.r ), g( _Source.g ), b( _Source.b ), a( _Alpha ) {}
inline constexpr rgba_colorf::rgba_colorf( const rgba_colorf & _Source, float _Alpha ) : r( _Source.r ), g( _Source.g ), b( _Source.b ), a( _Alpha ) {}

/* hsva_colorf */
inline constexpr hsva_colorf::hsva_colorf( float _Alpha ) : h( 0.0f ), s( 0.0f ), v( 0.0f ), a( _Alpha ) {}
inline constexpr hsva_colorf::hsva_colorf( float _H, float _S, float _V, float _A ) : h( _H ), s( _S ), v( _V ), a( _A ) {}

/* rgba_color8 */
inline constexpr rgba_color8::rgba_color8( unsigned char _Alpha ): r( 0 ), g( 0 ), b( 0 ), a( _Alpha ) {}
inline constexpr rgba_color8::rgba_color8( unsigned char _R, unsigned char _G, unsigned char _B, unsigned char _A ):
	r( _R ), g( _G ), b( _B ), a( _A ) {}
inline constexpr rgba_color8::rgba_color8( Colors::KnownColor _Known_Color, unsigned char _Alpha ):
	r( unsigned char( _Known_Color >> 16 ) ),
	g( unsigned char( _Known_Color >> 8  ) ),
	b( unsigned char( _Known_Color       ) ),
	a( _Alpha )
{}
inline constexpr rgba_color8::rgba_color8( const rgba_colorf & _Source ):
	r( unsigned char( _Source.r * 255.0f ) ),
	g( unsigned char( _Source.g * 255.0f ) ),
	b( unsigned char( _Source.b * 255.0f ) ),
	a( unsigned char( _Source.a * 255.0f ) )
{}
	
/* hsva_color8 */
inline constexpr hsva_color8::hsva_color8( unsigned char _Alpha ) : h( 0 ), s( 0 ), v( 0 ), a( _Alpha ) {}
inline constexpr hsva_color8::hsva_color8( unsigned char _H, unsigned char _S, unsigned char _V, unsigned char _A ): 
	h( _H ), s( _S ), v( _V ), a( _A ) {}


// this can be optimized for the architecture
inline constexpr unsigned int rgba_color8::pack()
{ return ( (unsigned int)r << 24 )|( (unsigned int)g << 16 )|( (unsigned int)b << 8 )|( (unsigned int)a ); }



/* *************************** (Color Arithmetic) ************************* */

/* rgba_colorf */
inline constexpr rgba_colorf rgba_colorf::operator+(const rgba_colorf&rhs)const{return rgba_colorf(this->r+rhs.r,this->g+rhs.g,this->b+rhs.b,this->a+rhs.a);}
inline constexpr rgba_colorf rgba_colorf::operator-(const rgba_colorf&rhs)const{return rgba_colorf(this->r-rhs.r,this->g-rhs.g,this->b-rhs.b,this->a-rhs.a);}
inline constexpr rgba_colorf rgba_colorf::operator*(const rgba_colorf&rhs)const{return rgba_colorf(this->r*rhs.r,this->g*rhs.g,this->b*rhs.b,this->a*rhs.a);}
inline constexpr rgba_colorf rgba_colorf::operator/(const rgba_colorf&rhs)const{return rgba_colorf(this->r/rhs.r,this->g/rhs.g,this->b/rhs.b,this->a/rhs.a);}

inline constexpr rgba_colorf rgba_colorf::operator*(const float&rhs)const{return rgba_colorf(this->r*rhs,this->g*rhs,this->b*rhs,this->a*rhs);}
inline constexpr rgba_colorf rgba_colorf::operator/(const float&rhs)const{return rgba_colorf(this->r/rhs,this->g/rhs,this->b/rhs,this->a/rhs);}

inline rgba_colorf&rgba_colorf::operator+=(const rgba_colorf&rhs){this->r+=rhs.r,this->g+=rhs.g,this->b+=rhs.b,this->a+=rhs.a;return*this;}
inline rgba_colorf&rgba_colorf::operator-=(const rgba_colorf&rhs){this->r-=rhs.r,this->g-=rhs.g,this->b-=rhs.b,this->a-=rhs.a;return*this;}
inline rgba_colorf&rgba_colorf::operator*=(const rgba_colorf&rhs){this->r*=rhs.r,this->g*=rhs.g,this->b*=rhs.b,this->a*=rhs.a;return*this;}
inline rgba_colorf&rgba_colorf::operator/=(const rgba_colorf&rhs){this->r/=rhs.r,this->g/=rhs.g,this->b/=rhs.b,this->a/=rhs.a;return*this;}

inline rgba_colorf&rgba_colorf::operator*=(const float&rhs){this->r*=rhs,this->g*=rhs,this->b*=rhs,this->a*=rhs;return*this;}
inline rgba_colorf&rgba_colorf::operator/=(const float&rhs){this->r/=rhs,this->g/=rhs,this->b/=rhs,this->a/=rhs;return*this;}



/* ************************** (Color Conversions) ************************* */

#include "hsv_conversions.inl"

inline constexpr rgb_colorf::rgb_colorf( const hsv_colorf & _Source ) : rgb_colorf()
	HSV_TO_RGB( r, g, b, _Source.h, _Source.s, _Source.v )

inline constexpr hsv_colorf::hsv_colorf( const rgb_colorf & _Source ) : hsv_colorf()
	RGB_TO_HSV( _Source.r, _Source.g, _Source.b, h, s, v )

inline constexpr rgba_colorf::rgba_colorf( const hsva_colorf & _Source ) : rgba_colorf( _Source.a )
	HSV_TO_RGB( r, g, b, _Source.h, _Source.s, _Source.v )

inline constexpr rgba_colorf::rgba_colorf( const hsv_colorf & _Source, float _Alpha ) : rgba_colorf( _Alpha )
	HSV_TO_RGB( r, g, b, _Source.h, _Source.s, _Source.v )

inline constexpr hsva_colorf::hsva_colorf( const rgba_colorf & _Source ) : hsva_colorf( _Source.a )
	RGB_TO_HSV( _Source.r, _Source.g, _Source.b, h, s, v )

inline constexpr hsva_colorf::hsva_colorf( const rgb_colorf & _Source, float _Alpha ) : hsva_colorf( _Alpha )
	RGB_TO_HSV( _Source.r, _Source.g, _Source.b, h, s, v )

#undef HSV_TO_RGB
#undef RGB_TO_HSV

_FISSION_BASE_PUBLIC_END

