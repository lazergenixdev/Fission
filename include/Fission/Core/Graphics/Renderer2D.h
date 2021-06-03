/**
*
* @file: Renderer2D.h
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

// todo: MESHES SUCK! improve interface

#pragma once
#include "Fission/config.h"
#include "Fission/Base/Math/Matrix.h"
#include "Fission/Base/Rect.h"
#include "Renderer.h"
#include "Graphics.h"
#include "Font.h"

namespace Fission
{
	enum class StrokeStyle {
		Center, Inside, Outside,
		Default = Center,
	};

	struct TextLayout {
		float width, height;
	};

	enum class BlendMode {
		Normal, Add, Source,
		Sub, Div, Mul,
		__count__
	};

	class Mesh 
	{
	public:
		FISSION_API Mesh( int vertex_count, int index_count, int color_count = 1 );
		FISSION_API Mesh( const Mesh & );
		FISSION_API ~Mesh();

		FISSION_API void push_color( color col );
		FISSION_API void push_vertex( base::vector2f position, int color_index );
		FISSION_API void push_index( uint32_t index );

		FISSION_API void set_color( uint32_t index, color new_color );

		FISSION_API uint32_t vertex_count() const;
		FISSION_API uint32_t index_count() const;
		FISSION_API uint32_t color_count() const;

	public:
		struct MeshData * m_Data;
	};

	class Renderer2D : public IRenderer
	{
	public:

		FISSION_API static std::unique_ptr<Renderer2D> Create( Graphics * pGraphics );

		virtual void FillRect( base::rectf rect, color color ) = 0;

		virtual void DrawRect( base::rectf rect, color color, float stroke_width, StrokeStyle stroke = StrokeStyle::Default ) = 0;

		virtual void FillTriangle( base::vector2f p0, base::vector2f p1, base::vector2f p2, color color ) = 0;

		virtual void FillTriangleGrad( base::vector2f p0, base::vector2f p1, base::vector2f p2, color c0, color c1, color c2 ) = 0;

		virtual void FillTriangleUV( base::vector2f p0, base::vector2f p1, base::vector2f p2, base::vector2f uv0, base::vector2f uv1, base::vector2f uv2, Resource::Texture2D * pTexture, color tint = Colors::White ) = 0;

		virtual void FillRectGrad( base::rectf rect, color color_topleft, color color_topright, color color_bottomleft, color color_bottomright ) = 0;

		virtual void FillRoundRect( base::rectf rect, float rad, color color ) = 0;

		virtual void DrawRoundRect( base::rectf rect, float rad, color color, float stroke_width, StrokeStyle stroke = StrokeStyle::Default ) = 0;

		virtual void DrawLine( base::vector2f start, base::vector2f end, color color, float stroke_width = 1.0f, StrokeStyle stroke = StrokeStyle::Default ) = 0;

		virtual void FillCircle( base::vector2f point, float radius, color color ) = 0;

		virtual void DrawCircle( base::vector2f point, float radius, color color, float stroke_width, StrokeStyle stroke = StrokeStyle::Default ) = 0;

		virtual void DrawCircle( base::vector2f point, float radius, color inner_color, color outer_color, float stroke_width, StrokeStyle stroke = StrokeStyle::Default ) = 0;

		virtual void FillArrow( base::vector2f start, base::vector2f end, float width, color color ) = 0;

		virtual void DrawImage( Resource::Texture2D * pTexture, base::rectf rect, base::rectf uv, color tint = Colors::White ) = 0;

		virtual void DrawImage( Resource::Texture2D * pTexture, base::rectf rect, color tint = Colors::White ) = 0;

		virtual void DrawMesh( const Mesh * m ) = 0;

		virtual void SelectFont( const Font * pFont ) = 0;

		virtual TextLayout DrawString( const wchar_t * wstr, base::vector2f pos, color color ) = 0;

		virtual TextLayout CreateTextLayout( const wchar_t * wstr ) = 0;

		virtual TextLayout DrawString( const char * str, base::vector2f pos, color color ) = 0;

		virtual TextLayout CreateTextLayout( const char * str ) = 0;

		virtual void SetBlendMode( BlendMode mode ) = 0;

		virtual void PushTransform( const base::matrix2x3f & transform ) = 0;

		virtual void PopTransform() = 0;

	//	virtual void PushClipRect( const rectf & rect ) = 0;

	//	virtual void PopClipRect() = 0;

	}; // class Fission::Renderer2D

} // namespace Fission
