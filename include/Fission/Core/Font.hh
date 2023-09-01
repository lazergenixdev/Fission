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
#include "Fission/Core/Graphics.hh"

//! @TODO: Cache Font Faces that are already loaded.
//! @TODO: System for picking language codepoints (用éäК)

__FISSION_BEGIN__

struct Glyph {
	rf32 uv; // Location in font atlas
	rf32 rc; // Location in pixels
	float advance;
};

struct Glyph_Table_Fast {
	Glyph fallback;
	Glyph glyphs[127-32]; // store all ascii characters

	Glyph const* lookup(u32 u) {
		if (u < 32 || u > 127) return &fallback;
		return &glyphs[u - 32];
	}
};
static constexpr int __sizeof_Glyph_Table_Fast = sizeof(Glyph_Table_Fast);
struct Glyph_Table_Standard {

};

struct Font {
	float height;

	// TODO: There will only every be two variants of this function, virtual is overkill
	virtual Glyph const* lookup(c32 codepoint) = 0;
	virtual void destroy() = 0;
};

struct Font_Simple_ASCII {
};

// Fonts where the character atlas's are always the same
struct Font_Static : public Font {
	VkDescriptorSet  texture;
	Glyph_Table_Fast table;
	VkImage          atlas_image;
	VmaAllocation    atlas_allocation;
	VkImageView      atlas_view;
	
	Font_Static() = default;

	void create(void const* ttf_data, size_t size, float height, VkDescriptorSet set, VkSampler sampler);

	virtual Glyph const* lookup(c32 codepoint) override;
	virtual void destroy() override;
};

// Fonts where new characters can be added to the atlas at runtime
struct Font_Dynamic {
//	Glyph_Table table;
//	FT_Face face;

};


static v2f32 bounding_box(Font* font, string s) {
	Glyph const* glyph = nullptr;
	v2f32 pos = {0, font->height};
	float max_width = 0.0f;

	FS_FOR(s.count) {
		auto c = s.data[i];
		glyph = font->lookup(c);
		if (c == '\n') {
			max_width = std::max(max_width, pos.x);
			pos.x = 0;
			pos.y += font->height;
			continue;
		}
		if (glyph) {
			pos.x += glyph->advance;
		}
	}

	return {std::max(max_width, pos.x), pos.y};
}

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