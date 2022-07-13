#pragma once
#include <Fission/Base/util/SmartPointer.hpp>
#include <Fission/Core/Graphics/Renderer2D.hh>

namespace Fission {


	class Renderer2DImpl : public IFRenderer2D
	{
	public:

		Renderer2DImpl() = default;

		virtual void OnCreate( IFGraphics * gfx, size2 _Viewport_Size ) override;

		virtual void OnRecreate( IFGraphics * gfx ) override;

		virtual void Destroy() override;

		virtual void OnResize( IFGraphics * gfx, size2 size ) override;

		// Inherited via Renderer2D
		virtual void Render() override;

		virtual void FillTriangle( v2f32 p0, v2f32 p1, v2f32 p2, color color ) override;

		virtual void FillTriangleGrad( v2f32 p0, v2f32 p1, v2f32 p2, color c0, color c1, color c2 ) override;

		virtual void FillTriangleUV( v2f32 p0, v2f32 p1, v2f32 p2, v2f32 uv0, v2f32 uv1, v2f32 uv2, Resource::IFTexture2D * pTexture, color tint ) override;

		virtual void FillRect( base::rectf rect, color color ) override;

		virtual void DrawRect( base::rectf rect, color color, float stroke_width, StrokeStyle stroke ) override;

		virtual void FillRectGrad( base::rectf rect, color color_topleft, color color_topright, color color_bottomleft, color color_bottomright ) override;

		virtual void FillRoundRect( base::rectf rect, float rad, color color ) override;

		virtual void DrawRoundRect( base::rectf rect, float rad, color color, float stroke_width, StrokeStyle stroke ) override;

		virtual void DrawLine( v2f32 start, v2f32 end, color color, float stroke_width, StrokeStyle stroke ) override;

		virtual void FillCircle( v2f32 point, float radius, color color ) override;

		virtual void DrawCircle( v2f32 point, float radius, color color, float stroke_width, StrokeStyle stroke ) override;

		virtual void DrawCircle( v2f32 point, float radius, color inner_color, color outer_color, float stroke_width, StrokeStyle stroke ) override;

		virtual void FillArrow( v2f32 start, v2f32 end, float width, color color ) override;

		virtual void DrawImage( Resource::IFTexture2D * pTexture, base::rectf rect, base::rectf uv, color tint ) override;

		virtual void DrawImage( Resource::IFTexture2D * pTexture, base::rectf rect, color tint ) override;

		virtual void DrawMesh( const Mesh * m ) override;

		virtual void SelectFont( const Font * pFont ) override;

		virtual TextLayout DrawString( const char * str, v2f32 pos, color color ) override;

		virtual TextLayout DrawString( string_view sv, v2f32 pos, color color ) override;

		virtual TextLayout CreateTextLayout( const char * str ) override;

		virtual TextLayout CreateTextLayout( const char * str, uint32_t length ) override;

		virtual void SetBlendMode( BlendMode mode ) override;

		virtual void PushTransform( m23 const& transform ) override;

		virtual void PopTransform() override;

	private:
		void _set_accumulated_transform();

	private:
		m23 m_accTransform = m23::Identity();
		dynamic_buffer<m23> m_TransformStack;

	private:
		IFGraphics * m_pGraphics = nullptr;

		fsn_ptr<Resource::IFVertexBuffer>   m_pVertexBuffer;
		fsn_ptr<Resource::IFIndexBuffer>    m_pIndexBuffer;
		fsn_ptr<Resource::IFConstantBuffer> m_pTransformBuffer;
		fsn_ptr<Resource::IFShader>			m_pShader;
		fsn_ptr<Resource::IFBlender>		m_pBlenders[(size_t)BlendMode::__count__];
		Resource::IFBlender *				m_pUseBlender = nullptr;

		const Font *						m_pSelectedFont = nullptr;

	private:
		struct vertex {
			vertex() = default;
			vertex( v2f32 pos, color c ) : pos( pos ), tc( -1.0f, 0.0f ), color( c ) {}
			vertex( v2f32 pos, v2f32 tc, color c ) : pos( pos ), tc( tc ), color( c ) {}

			v2f32 pos, tc;
			color color;
		};

		void SetTexture( Resource::IFTexture2D * tex );

		struct DrawData 
		{
			DrawData( Renderer2DImpl * parent, uint32_t vc, uint32_t ic );

			void AddRectFilled( v2f32 tl, v2f32 tr, v2f32 bl, v2f32 br, color c );
			void AddRectFilled( base::rectf rect, color c );
			void AddRectFilled( base::rectf rect, color tl, color tr, color bl, color br );
			void AddRectFilledUV( base::rectf rect, base::rectf uv, color c );
			void AddRect( base::rectf rect, color color, float stroke_width, StrokeStyle stroke );

			void AddRoundRectFilled( base::rectf rect, float rad, color c );
			void AddRoundRect( base::rectf rect, float rad, color color, float stroke_width, StrokeStyle stroke );

			void AddMesh( const Mesh * mesh );
			void AddCircleFilled( v2f32 center, float rad, color c );
			void AddCircle( v2f32 center, float rad, color inc, color outc, float stroke_width, StrokeStyle stroke );
			void AddTriangle( v2f32 p0, v2f32 p1, v2f32 p2, color c0, color c1, color c2 );
			void AddTriangleUV( v2f32 p0, v2f32 p1, v2f32 p2, v2f32 uv0, v2f32 uv1, v2f32 uv2, color c );
			void AddLine( v2f32 start, v2f32 end, float stroke, color startColor, color endColor );


			uint32_t vtxCount = 0, vtxStart;
			uint32_t idxCount = 0, idxStart;

			Resource::IFTexture2D * Texture = nullptr;

			vertex *	pVtxData = nullptr;
			uint32_t *	pIdxData = nullptr;

			struct sincos { float sin, cos; };

			// TODO: Create "mips" for this cache to reduce unnecessary geometry
			static std::vector<sincos> TrigCache;

			m23 const* mat;
		};

		std::vector<DrawData> m_DrawBuffer;

		vertex * vertex_data = nullptr;
		uint32_t * index_data = nullptr;

		static constexpr int vertex_max_count = 100000;
		static constexpr int index_max_count = 200000;

		size2 _viewport_size;
	};

}
