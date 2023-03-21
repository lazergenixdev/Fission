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
#include <Fission/Core/Graphics/Renderer.hh>
#include <Fission/Base/Math/Vector.hpp>
#include <Fission/Base/Color.hpp>

namespace Fission
{
	struct TextRenderer : public Renderer
	{
		//! @return: width and height of text bounds in a vector
		virtual v2f32 text_bounds    (struct Font*    font, const char* text) = 0;
		virtual v2f32 text_bounds_ui (struct UIFont*  font, const chr*  text) = 0;
		virtual v2f32 text_bounds_sdf(struct SDFFont* font, const char* text, float size) = 0;

		//! @return: width and height of text bounds in a vector
		virtual v2f32 add_text    (struct Font*    font, const char* text, v2f32 pos, color color = colors::White) = 0;
		virtual v2f32 add_text_ui (struct UIFont*  font, const chr*  text, v2f32 pos, float size, color color = colors::White) = 0;
		virtual v2f32 add_text_sdf(struct SDFFont* font, const char* text, v2f32 pos, float size, color color = colors::White) = 0;

		virtual void render() = 0;
	};

	FISSION_API void CreateTextRenderer( TextRenderer** ppTextRenderer );

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