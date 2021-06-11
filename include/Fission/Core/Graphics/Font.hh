/**
*
* @file: Font.h
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

// todo: complete redesign of font manager; works great now, but has many
//	many issues when it comes to trying to reuse font faces and memory usage

#pragma once
#include "Fission/Base/Rect.h"
#include "Fission/Core/Graphics/Bindable.hh"

namespace Fission {

	struct Font
	{
	public:
		struct Glyph {
			base::rectf rc;
			base::vector2f offset, size;
			float advance;
		};

	public:
		struct CreateOptions {
			int unused = 0;
		};

		virtual Resource::IFTexture2D * GetTexture2D() const = 0;

		virtual const Glyph * GetGylph( wchar_t ch ) const = 0;

		virtual const Glyph * GetGylphOutline( wchar_t ch ) const = 0;

		virtual float GetSize() const = 0;

	//	virtual void SetSize(float) const = 0;

		virtual ~Font() = default;
	};

	struct FontFace
	{
		virtual std::unique_ptr<Font> CreateFont( float size, const Font::CreateOptions & options = {} );

		virtual ~FontFace() = default;

		FISSION_API static std::unique_ptr<FontFace> FontFaceFromFile( const wchar_t * file_path );

		FISSION_API static std::unique_ptr<FontFace> FontFaceFromMemory( const void * pdata, size_t size );
	};

	class FontManager
	{
	public:
	
		FISSION_API static Font * GetFont( const char * key );

		// "$debug" ----- used by the debug layer
		// "$console" --- used by the console layer
		// "$ui" -------- used by the ui layer
		FISSION_API static void SetFont( const char * key, const std::filesystem::path & filepath, float pxsize, struct IFGraphics * gfx );

		FISSION_API static void SetFont( const char * key, const void * pdata, size_t size, float pxsize, struct IFGraphics * gfx );

		FISSION_API static void DelFont( const char * key );

	}; // class Fission::FontManager

} // namespace Fission
