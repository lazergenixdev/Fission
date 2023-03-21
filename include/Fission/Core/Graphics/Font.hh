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
#include "Fission/Base/Rect.hpp"
#include "Fission/Base/String.hpp"
#include "Fission/Core/Graphics/Bindable.hh"

//! @TODO: Cache Font Faces that are already loaded.
//! @TODO: System for picking language codepoints (用éäК)

namespace Fission {

	enum FontType {
		Basic, // Basic Font Atlas implementation
		SDF,   // Scalable fonts using signed-distance fields
		UI     // Font that supports most of Unicode (hopefully in the future)
	};

	//! Basic Font Type
	struct Font : public ManagedObject
	{
	public:
		struct Glyph {
			rf32 uv; // Location in font atlas
			rf32 rc; // Location in pixels
			float advance;
		};

		struct CreateInfo {
			const void* fontfile;
			u64 fontfilesize;
			float size;
		};
	public:

		//! @note This function must be called AFTER graphics is initialized,
		//!			font atlas is saved to the GPU, not to system memory
		FISSION_API static Font* Create( const CreateInfo& _Info );

		virtual gfx::Texture2D * get_atlas() const = 0;

		virtual const Glyph * lookup( chr _Codepoint ) const = 0;

		virtual float height() const = 0;

		virtual void resize(float _New_Size) = 0;
	};

	//! UI Font Type
	struct UIFont : public Font
	{
	public:
		struct CreateInfo {
			const void* fontfile;
			u64 fontfilesize;
			const void* emojifile;
			u64 emojifilesize;
			float size;
		};

	public:
		//! @note This function must be called AFTER graphics is initialized
		//!			font atlas is saved to the GPU, not to system memory
		FISSION_API static UIFont* Create( const CreateInfo& _Info );

		virtual float size() const = 0;

		virtual const Glyph* fallback() const = 0;

		virtual const Glyph* lookup_emoji(const chr * codepoints, int& advance) const = 0;

		virtual gfx::Texture2D * get_emoji_atlas() const = 0;
	};

	//! SDF Font Type
	struct SDFFont : public Font
	{
	public:
		struct CreateInfo {
			const void* glyph_data;
			u64         glyph_data_size;
			const char* atlas_filename;
		};

	public:
		//! @note This function must be called AFTER graphics is initialized
		//!			font atlas is saved to the GPU, not to system memory
		FISSION_API static SDFFont* Create( const CreateInfo& _Info );
	};

} // namespace Fission

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