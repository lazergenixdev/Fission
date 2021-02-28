#pragma once
#include "Bindable.h"

namespace lazer {

	interface Font
	{
	public:
		struct Glyph {
			rectf rc;
			vec2f offset, size;
			float advance;
		};

	public:
		struct CreateOptions {
			int unused = 0;
		};

		virtual Resource::Texture2D * GetTexture2D() const = 0;

		virtual const Glyph * GetGylph( wchar_t ch ) const = 0;

		virtual const Glyph * GetGylphOutline( wchar_t ch ) const = 0;

		virtual float GetSize() const = 0;

	//	virtual void SetSize(float) const = 0;

		virtual ~Font() = default;
	};

	interface FontFace
	{
		virtual std::unique_ptr<Font> CreateFont( float size, const Font::CreateOptions & options = {} );

		virtual ~FontFace() = default;

		LAZER_API static std::unique_ptr<FontFace> FontFaceFromFile( const wchar_t * file_path );

		LAZER_API static std::unique_ptr<FontFace> FontFaceFromMemory( const void * pdata, size_t size );
	};

	class FontManager
	{
	public:
	
		LAZER_API static Font * GetFont( const char * key );

		// "$debug" ----- used by the debug layer
		// "$console" --- used by the console layer
		// "$ui" -------- used by the ui layer
		LAZER_API static void SetFont( const char * key, const file::path & filepath, float pxsize );

		LAZER_API static void SetFont( const char * key, const void * pdata, size_t size, float pxsize );

	};

}
