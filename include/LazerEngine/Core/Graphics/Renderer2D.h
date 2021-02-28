#pragma once
#include "LazerEngine/config.h"
#include "Renderer.h"
#include "Graphics.h"
#include "Font.h"
#include "lazer/matrix.h"

namespace lazer
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
		LAZER_API Mesh( int vertex_count, int index_count, int color_count = 1 );
		LAZER_API Mesh( const Mesh & );
		LAZER_API ~Mesh();

		LAZER_API void push_color( color col );
		LAZER_API void push_vertex( vec2f position, int color_index );
		LAZER_API void push_index( uint32_t index );

		LAZER_API void set_color( uint32_t index, color new_color );

		LAZER_API uint32_t vertex_count() const;
		LAZER_API uint32_t index_count() const;
		LAZER_API uint32_t color_count() const;

	public:
		struct MeshData * m_Data;
	};

	class Renderer2D : public IRenderer
	{
	public:

		LAZER_API static std::unique_ptr<Renderer2D> Create( Graphics * pGraphics );

		virtual void FillRect( rectf rect, colorf color ) = 0;

		virtual void DrawRect( rectf rect, colorf color, float stroke_width, StrokeStyle stroke = StrokeStyle::Default ) = 0;

		virtual void FillTriangle( vec2f p0, vec2f p1, vec2f p2, colorf color ) = 0;

		virtual void FillTriangleUV( vec2f p0, vec2f p1, vec2f p2, vec2f uv0, vec2f uv1, vec2f uv2, Resource::Texture2D * pTexture, colorf tint = Colors::White ) = 0;

		virtual void FillRectGrad( rectf rect, colorf color_topleft, colorf color_topright, colorf color_bottomleft, colorf color_bottomright ) = 0;

		virtual void FillRoundRect( rectf rect, float rad, colorf color ) = 0;

		virtual void DrawRoundRect( rectf rect, float rad, colorf color, float stroke_width, StrokeStyle stroke = StrokeStyle::Default ) = 0;

		virtual void DrawLine( vec2f start, vec2f end, colorf color, float stroke_width = 1.0f, StrokeStyle stroke = StrokeStyle::Default ) = 0;

		virtual void FillCircle( vec2f point, float radius, colorf color ) = 0;

		virtual void DrawCircle( vec2f point, float radius, colorf color, float stroke_width, StrokeStyle stroke = StrokeStyle::Default ) = 0;

		virtual void FillArrow( vec2f start, vec2f end, float width, colorf color ) = 0;

		virtual void DrawImage( Resource::Texture2D * pTexture, rectf rect, rectf uv, colorf tint = Colors::White ) = 0;

		virtual void DrawImage( Resource::Texture2D * pTexture, rectf rect, colorf tint = Colors::White ) = 0;

		virtual void DrawMesh( const Mesh * m ) = 0;

		virtual void SelectFont( const Font * pFont ) = 0;

		virtual TextLayout DrawString( const wchar_t * wstr, vec2f pos, colorf color ) = 0;

		virtual TextLayout CreateTextLayout( const wchar_t * wstr ) = 0;

		virtual TextLayout DrawString( const char * str, vec2f pos, colorf color ) = 0;

		virtual TextLayout CreateTextLayout( const char * str ) = 0;

		virtual void SetBlendMode( BlendMode mode ) = 0;

		virtual void PushTransform( const mat3x2f & transform ) = 0;

		virtual void PopTransform() = 0;

	//	virtual void PushClipRect( const rectf & rect ) = 0;

	//	virtual void PopClipRect() = 0;

	}; // class lazer::Renderer2D

} // namespace lazer
