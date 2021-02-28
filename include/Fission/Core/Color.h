#pragma once
#include "Fission/config.h"

// todo:
// [ ] - make a saturate function
// [ ] - use saturate so color values are never >1.0
// maybe add to lazerlib?

namespace Fission
{

/* Colors which use single-persision floating point */
	struct colorf;
	struct colorf_hsv;

/* Colors which use 8-bit integers */
	struct coloru;
	struct coloru_hsv;

	// Select defualt structure to use for color
	using color = colorf;

	namespace Colors {
		enum KnownColor : uInt32 {
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

		static constexpr color make_color( const color & color, float value, float alpha = 1.0f );

	} // namespace Fission::Colors


	struct colorf_hsv { 
		float h, s, v, a; 

		constexpr colorf_hsv( const colorf_hsv & src ) = default;

		constexpr colorf_hsv() : h( 0.0f ), s( 0.0f ), v( 0.0f ), a( 1.0f ) {}

		constexpr colorf_hsv( float _h, float _s, float _v, float _a = 1.0f )
			: h( _h ), s( _s ), v( _v ), a( _a )
		{}

		colorf_hsv( const coloru_hsv & src );

		colorf_hsv( const colorf & src );
	}; // struct Fission::colorf_hsv

	struct coloru_hsv { 
		unsigned char h, s, v, a; 
	}; // struct Fission::coloru_hsv

	struct colorf {
		float r, g, b, a;

		constexpr colorf( const colorf & ) = default;

		constexpr colorf() : r( 0.0f ), g( 0.0f ), b( 0.0f ), a( 1.0f ) {}

		constexpr colorf( float _r, float _g, float _b, float _a = 1.0f )
			: r( _r ), g( _g ), b( _b ), a( _a )
		{}

		constexpr colorf( const colorf & src, float _a )
			: r( src.r ), g( src.g ), b( src.b ), a( _a )
		{}

		constexpr colorf( Colors::KnownColor known_col, float _a = 1.0f ):
			r( float( known_col >> 16 ) / 255.0f ),
			g( float( ( known_col & 0x00ff00 ) >> 8 ) / 255.0f ),
			b( float( known_col & 0xff ) / 255.0f ),
			a( _a )
		{}

		colorf( const coloru & src );

		colorf( const coloru & src, float _a );

		colorf( const colorf_hsv & src );

		colorf operator+( const colorf & rhs ) const {
			return colorf( r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a );
		}
		colorf operator-( const colorf & rhs ) const {
			return colorf( r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a );
		}
		colorf operator*( const colorf & rhs ) const {
			return colorf( r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a );
		}
		colorf operator*( const float rhs ) const {
			return colorf( r * rhs, g * rhs, b * rhs, a * rhs );
		}
	}; // struct Fission::colorf

	struct coloru {
		unsigned char r, g, b, a;

		constexpr coloru() : r( 0 ), g( 0 ), b( 0 ), a( 255u ) {}

		constexpr coloru( unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255 )
			: r( r ), g( g ), b( b ), a( a )
		{}

		constexpr coloru( Colors::KnownColor known_col, unsigned char _a = 255 )
			: r( known_col >> 16 ), g( known_col >> 8 ), b( known_col ), a( _a )
		{}

		constexpr coloru( uint32_t argb ) :
			r( argb >> 16 ), g( argb >> 8 ), b( argb ), a( argb >> 24 )
		{}

		coloru( const coloru & ) = default;

		coloru( const coloru & src, unsigned char _a )
			: r( src.r ), g( src.g ), b( src.b ), a( _a )
		{}

		coloru( const colorf & src );

		coloru( const colorf & src, unsigned char _a );
	}; // struct Fission::coloru


	//*********************** Begin Conversion Functions ***********************//

	inline colorf::colorf( const coloru & src ) :
		r( (float)src.r / 255.0f ), 
		g( (float)src.g / 255.0f ), 
		b( (float)src.b / 255.0f ), 
		a( (float)src.a / 255.0f )
	{}
	
	inline colorf::colorf( const coloru & src, float _a ) :
		r( (float)src.r / 255.0f ),
		g( (float)src.g / 255.0f ),
		b( (float)src.b / 255.0f ),
		a( _a )
	{}

	inline colorf::colorf( const colorf_hsv & src ) : a( src.a )
	{
		// code i copied from stackoverflow LOL
		float hh, p, q, t, ff;
		long i;

		if( src.s <= 0.0f ) {
			r = src.v;
			g = src.v;
			b = src.v;
			return;
		}
		hh = src.h;
		if( hh >= 1.0f ) hh = 0.0f;
		hh *= 6.0f;
		i = (long)hh;
		ff = hh - i;
		p = src.v * ( 1.0f - src.s );
		q = src.v * ( 1.0f - src.s * ff );
		t = src.v * ( 1.0f - src.s * ( 1.0f - ff ) );

		switch( i ) {
		case 0: r = src.v, g = t, b = p; break;
		case 1: r = q, g = src.v, b = p; break;
		case 2: r = p, g = src.v, b = t; break;
		case 3: r = p, g = q, b = src.v; break;
		case 4: r = t, g = p, b = src.v; break;
		default:
		case 5: r = src.v, g = p, b = q; break;
		}
	}

	inline coloru::coloru( const colorf & src ) :
		r( (unsigned char)( src.r * 255.0f + 0.5f ) ),
		g( (unsigned char)( src.g * 255.0f + 0.5f ) ),
		b( (unsigned char)( src.b * 255.0f + 0.5f ) ),
		a( (unsigned char)( src.a * 255.0f + 0.5f ) )
	{}

	inline coloru::coloru( const colorf & src, unsigned char _a ) :
		r( (unsigned char)( src.r * 255.0f + 0.5f ) ),
		g( (unsigned char)( src.g * 255.0f + 0.5f ) ),
		b( (unsigned char)( src.b * 255.0f + 0.5f ) ),
		a( _a )
	{}


	inline colorf_hsv::colorf_hsv( const coloru_hsv & src ) :
		h( (float)src.h / 255.0f ),
		s( (float)src.s / 255.0f ),
		v( (float)src.v / 255.0f ),
		a( (float)src.a / 255.0f )
	{}

	inline colorf_hsv::colorf_hsv( const colorf & src ) : a( src.a )
	{
		// more code i copied from stackoverflow LOL
		float min, max, delta;

		min = src.r < src.g ? src.r : src.g;
		min = min < src.b ? min : src.b;

		max = src.r > src.g ? src.r : src.g;
		max = max > src.b ? max : src.b;

		v = max;
		delta = max - min;

		if( delta < 0.00001 ) {
			s = 0.0f, h = 0.0f;
			return;
		}

		if( max > 0.0f ) {
			s = ( delta / max );
		} else {
			s = 0.0f;
			h = NAN;
			return;
		}

		if( src.r >= max )
			h = ( src.g - src.b ) / delta;
		else
			if( src.g >= max ) h = 2.0f + ( src.b - src.r ) / delta;
			else			   h = 4.0f + ( src.r - src.g ) / delta;

		if( h < 0.0f ) h += 1.0f;
	}

	//*********************** End Conversion Functions ***********************//



	static constexpr color Colors::make_color( const color & c, float value, float alpha ) 
	{
		return color( c * value, alpha );
	}


} // namespace Fission

