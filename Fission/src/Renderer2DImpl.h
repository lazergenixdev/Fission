#pragma once
#include "Fission/Core/Graphics/Renderer2D.h"

namespace Fission {


	class Renderer2DImpl : public Renderer2D
	{
	public:

		Renderer2DImpl( Graphics * pGraphics );

		~Renderer2DImpl() noexcept;

		// Inherited via Renderer2D
		virtual void Render() override;

		virtual void FillTriangle( base::vector2f p0, base::vector2f p1, base::vector2f p2, color color ) override;

		virtual void FillTriangleGrad( base::vector2f p0, base::vector2f p1, base::vector2f p2, color c0, color c1, color c2 ) override;

		virtual void FillTriangleUV( base::vector2f p0, base::vector2f p1, base::vector2f p2, base::vector2f uv0, base::vector2f uv1, base::vector2f uv2, Resource::Texture2D * pTexture, color tint ) override;

		virtual void FillRect( rectf rect, color color ) override;

		virtual void DrawRect( rectf rect, color color, float stroke_width, StrokeStyle stroke ) override;

		virtual void FillRectGrad( rectf rect, color color_topleft, color color_topright, color color_bottomleft, color color_bottomright ) override;

		virtual void FillRoundRect( rectf rect, float rad, color color ) override;

		virtual void DrawRoundRect( rectf rect, float rad, color color, float stroke_width, StrokeStyle stroke ) override;

		virtual void DrawLine( base::vector2f start, base::vector2f end, color color, float stroke_width, StrokeStyle stroke ) override;

		virtual void FillCircle( base::vector2f point, float radius, color color ) override;

		virtual void DrawCircle( base::vector2f point, float radius, color color, float stroke_width, StrokeStyle stroke ) override;

		virtual void DrawCircle( base::vector2f point, float radius, color inner_color, color outer_color, float stroke_width, StrokeStyle stroke ) override;

		virtual void FillArrow( base::vector2f start, base::vector2f end, float width, color color ) override;

		virtual void DrawImage( Resource::Texture2D * pTexture, rectf rect, rectf uv, color tint ) override;

		virtual void DrawImage( Resource::Texture2D * pTexture, rectf rect, color tint ) override;

		virtual void DrawMesh( const Mesh * m ) override;

		virtual void SelectFont( const Font * pFont ) override;

		virtual TextLayout DrawString( const wchar_t * wstr, base::vector2f pos, color color ) override;

		virtual TextLayout CreateTextLayout( const wchar_t * wstr ) override;

		virtual TextLayout DrawString( const char * str, base::vector2f pos, color color ) override;

		virtual TextLayout CreateTextLayout( const char * str ) override;

		virtual void SetBlendMode( BlendMode mode ) override;

		virtual void PushTransform( const mat3x2f & transform ) override;

		virtual void PopTransform() override;

	private:
		void _set_accumulated_transform();

	private:
		mat3x2f m_accTransform = mat3x2f::Identity();
		std::vector<mat3x2f> m_TransformStack;

	private:
		Graphics * m_pGraphics;

		std::unique_ptr<Resource::VertexBuffer>		m_pVertexBuffer;
		std::unique_ptr<Resource::IndexBuffer>		m_pIndexBuffer;
		std::unique_ptr<Resource::Shader>			m_pShader;
		std::unique_ptr<Resource::Blender>			m_pBlenders[(size_t)BlendMode::__count__];
		Resource::Blender *							m_pUseBlender;

		const Font *								m_pSelectedFont = nullptr;

	private:
		struct vertex {
			vertex() = default;
			vertex( base::vector2f pos, color c ) : pos( pos ), tc( -1.0f, 0.0f ), color( c ) {}
			vertex( base::vector2f pos, base::vector2f tc, color c ) : pos( pos ), tc( tc ), color( c ) {}

			base::vector2f pos, tc;
			color color;
		};

		void SetTexture( Resource::Texture2D * tex );

		struct DrawData 
		{
			DrawData( Renderer2DImpl * parent, uint32_t vc, uint32_t ic );

			void AddRectFilled( base::vector2f tl, base::vector2f tr, base::vector2f bl, base::vector2f br, color c );
			void AddRectFilled( rectf rect, color c );
			void AddRectFilled( rectf rect, color tl, color tr, color bl, color br );
			void AddRectFilledUV( rectf rect, rectf uv, color c );
			void AddRect( rectf rect, color color, float stroke_width, StrokeStyle stroke );

			void AddRoundRectFilled( rectf rect, float rad, color c );
			void AddRoundRect( rectf rect, float rad, color color, float stroke_width, StrokeStyle stroke );

			void AddMesh( const Mesh * mesh );
			void AddCircleFilled( base::vector2f center, float rad, color c );
			void AddCircle( base::vector2f center, float rad, color inc, color outc, float stroke_width, StrokeStyle stroke );
			void AddTriangle( base::vector2f p0, base::vector2f p1, base::vector2f p2, color c0, color c1, color c2 );
			void AddTriangleUV( base::vector2f p0, base::vector2f p1, base::vector2f p2, base::vector2f uv0, base::vector2f uv1, base::vector2f uv2, color c );
			void AddLine( base::vector2f start, base::vector2f end, float stroke, color startColor, color endColor );


			uint32_t vtxCount = 0, vtxStart;
			uint32_t idxCount = 0, idxStart;

			Resource::Texture2D * Texture = nullptr;

			vertex *	pVtxData = nullptr;
			uint32_t *	pIdxData = nullptr;

			struct sincos { float sin, cos; };
			static std::vector<sincos> TrigCache;

			const mat3x2f * mat;
		};

		std::vector<DrawData> m_DrawBuffer;

		vertex * vertex_data = nullptr;
		uint32_t * index_data = nullptr;

		static constexpr int vertex_max_count = 100000;
		static constexpr int index_max_count = 200000;
	};

}
