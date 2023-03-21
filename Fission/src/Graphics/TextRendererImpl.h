#pragma once
#include <Fission/Base/util/SmartPointer.hpp>
#include <Fission/Core/Graphics/TextRenderer.hh>
#include <Fission/Core/Graphics/Bindable.hh>
#include <Fission/Core/Graphics/Font.hh>

namespace Fission {


	class TextRendererImpl : public TextRenderer
	{
	public:

		TextRendererImpl() = default;

		virtual void OnCreate( Graphics * gfx, size2 _Viewport_Size ) override;
		virtual void OnRecreate( Graphics * gfx ) override;
		virtual void OnResize( Graphics * gfx, size2 size ) override;

		virtual void Destroy() override;

		virtual v2f32 text_bounds( struct Font* font, const char* text ) override;
		virtual v2f32 text_bounds_ui( struct UIFont* font, const chr* text ) override;
		virtual v2f32 text_bounds_sdf( struct SDFFont* font, const char* text, float size ) override;
		virtual v2f32 add_text( struct Font* font, const char* text, v2f32 pos, color color ) override;
		virtual v2f32 add_text_ui( struct UIFont* font, const chr* text, v2f32 pos, float size, color color ) override;
		virtual v2f32 add_text_sdf( struct SDFFont* font, const char* text, v2f32 pos, float size, color color ) override;
		virtual void render() override;

	private:
		Graphics * m_pGraphics = nullptr;

		fsn_ptr<gfx::VertexBuffer>   m_pVertexBuffer;
		fsn_ptr<gfx::IndexBuffer>    m_pIndexBuffer;
		fsn_ptr<gfx::ConstantBuffer> m_pTransformBuffer;
		fsn_ptr<gfx::Blender>		 m_pBlender;
		fsn_ptr<gfx::Sampler>		 m_pSampler;

		fsn_ptr<gfx::Shader>	     m_pShader;
		fsn_ptr<gfx::Shader>	     m_pSDF_Shader;

		FontType m_Type;

	private:
		struct vertex {
			v2f32 pos, tc;
			color color;
		};

		struct DrawData 
		{
			inline void add_glyph( const Font::Glyph* g, const v2f32& origin, const float& scale, const color& color );

			uint32_t vtxCount = 0;
			uint32_t idxCount = 0;

			gfx::Texture2D * Texture = nullptr;

			vertex *	pVtxData = nullptr;
			uint32_t *	pIdxData = nullptr;
		};

		DrawData f_DrawData; // font
		DrawData e_DrawData; // emoji

		static constexpr int vertex_max_count = 10000;
		static constexpr int index_max_count = vertex_max_count * 6 / 4; // every quad has 4 vertices and 6 indices
	};

}
