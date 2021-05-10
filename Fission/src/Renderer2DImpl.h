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

		virtual void FillTriangle( vec2f p0, vec2f p1, vec2f p2, colorf color ) override;

		virtual void FillTriangleGrad( vec2f p0, vec2f p1, vec2f p2, colorf c0, colorf c1, colorf c2 ) override;

		virtual void FillTriangleUV( vec2f p0, vec2f p1, vec2f p2, vec2f uv0, vec2f uv1, vec2f uv2, Resource::Texture2D * pTexture, colorf tint ) override;

		virtual void FillRect( rectf rect, colorf color ) override;

		virtual void DrawRect( rectf rect, colorf color, float stroke_width, StrokeStyle stroke ) override;

		virtual void FillRectGrad( rectf rect, colorf color_topleft, colorf color_topright, colorf color_bottomleft, colorf color_bottomright ) override;

		virtual void FillRoundRect( rectf rect, float rad, colorf color ) override;

		virtual void DrawRoundRect( rectf rect, float rad, colorf color, float stroke_width, StrokeStyle stroke ) override;

		virtual void DrawLine( vec2f start, vec2f end, colorf color, float stroke_width, StrokeStyle stroke ) override;

		virtual void FillCircle( vec2f point, float radius, colorf color ) override;

		virtual void DrawCircle( vec2f point, float radius, colorf color, float stroke_width, StrokeStyle stroke ) override;

		virtual void FillArrow( vec2f start, vec2f end, float width, colorf color ) override;

		virtual void DrawImage( Resource::Texture2D * pTexture, rectf rect, rectf uv, colorf tint ) override;

		virtual void DrawImage( Resource::Texture2D * pTexture, rectf rect, colorf tint ) override;

		virtual void DrawMesh( const Mesh * m ) override;

		virtual void SelectFont( const Font * pFont ) override;

		virtual TextLayout DrawString( const wchar_t * wstr, vec2f pos, colorf color ) override;

		virtual TextLayout CreateTextLayout( const wchar_t * wstr ) override;

		virtual TextLayout DrawString( const char * str, vec2f pos, colorf color ) override;

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
			vertex( vec2f pos, colorf c ) : pos( pos ), tc( -1.0f, 0.0f ), color( c ) {}
			vertex( vec2f pos, vec2f tc, colorf c ) : pos( pos ), tc( tc ), color( c ) {}

			vec2f pos, tc;
			colorf color;
		};

		void SetTexture( Resource::Texture2D * tex );

		struct DrawData 
		{
			DrawData( Renderer2DImpl * parent, uint32_t vc, uint32_t ic );

			void AddRectFilled( vec2f tl, vec2f tr, vec2f bl, vec2f br, color c );
			void AddRectFilled( rectf rect, color c );
			void AddRectFilled( rectf rect, color tl, color tr, color bl, color br );
			void AddRectFilledUV( rectf rect, rectf uv, color c );
			void AddRect( rectf rect, colorf color, float stroke_width, StrokeStyle stroke );
			void AddMesh( const Mesh * mesh );
			void AddCircleFilled( vec2f center, float rad, colorf c );
			void AddTriangle( vec2f p0, vec2f p1, vec2f p2, colorf c0, colorf c1, colorf c2 );
			void AddTriangleUV( vec2f p0, vec2f p1, vec2f p2, vec2f uv0, vec2f uv1, vec2f uv2, colorf c );
			void AddLine( vec2f start, vec2f end, float stroke, colorf startColor, colorf endColor );


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
