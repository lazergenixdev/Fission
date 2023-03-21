#include "TextRendererImpl.h"
#include <Fission/Base/Exception.hpp>
#include <Fission/Core/Graphics.hh>
#include <Fission/Core/Graphics/Font.hh>

namespace Fission
{
	static string_view font_emoji_shader =
		"cbuffer Transform:register(b0){matrix screen;}struct VS_OUT{float2 tc"
		":TexCoord;float4 color:Color;float4 pos:SV_Position;};VS_OUT vs_main("
		"float2 pos:Position,float2 tc:TexCoord,float4 color:Color){VS_OUT vso"
		";vso.pos=mul(float4(pos,1.0,1.0),screen);vso.tc=tc;vso.color=color;"
		"return vso;}Texture2D tex;SamplerState ss;float4 ps_main(float2 tc:"
		"TexCoord,float4 color:Color):SV_Target{return tex.Sample(ss,tc)*color;}";

	static string_view msdf_shader =
		"cbuffer Transform:register(b0){matrix screen;}struct VS_OUT{float2 tc"
		":TexCoord;float4 color:Color;float4 pos:SV_Position;};VS_OUT vs_main("
		"float2 pos:Position,float2 tc:TexCoord,float4 color:Color){VS_OUT vso"
		";vso.pos=mul(float4(pos,1.0,1.0),screen);vso.tc=tc;vso.color=color;"
		"return vso;}Texture2D tex;SamplerState ss;float median(float a,"
		"float b,float c){return max(min(a,b),min(max(a,b),c));}float4 ps_main"
		"(float2 tc:TexCoord,float4 color:Color):SV_TARGET{float3 dist=tex."
		"Sample(ss,tc).rgb;float d=median(dist.r,dist.g,dist.b)-0.5;float w="
		"clamp(d/fwidth(d)+0.5,0.0,1.0);float4 outside=float4(0,0,0,0);"
		"float4 inside=color;float4 a=lerp(outside,inside,w);return a;}";

	static size2 _viewport_size;

	void CreateTextRenderer( TextRenderer** ppTextRenderer )
	{
		*ppTextRenderer = new TextRendererImpl;
	}

	void TextRendererImpl::OnCreate( Graphics * gfx, size2 _Viewport_Size )
	{
		// Allocate aligned memory for faster access
		f_DrawData.pVtxData = (vertex *)_aligned_malloc( vertex_max_count * sizeof vertex, 32 );
		f_DrawData.pIdxData = (uint32_t *)_aligned_malloc( index_max_count * sizeof uint32_t, 32 );

		_viewport_size = _Viewport_Size;

		OnRecreate( gfx );
	}

	void TextRendererImpl::OnRecreate( Graphics * gfx )
	{
		m_pGraphics = gfx;
		using namespace gfx;
		using namespace gfx::VertexLayoutTypes;

		auto vl = VertexLayout{};
		vl.Append( Float2, "Position" );
		vl.Append( Float2, "TexCoord" );
		vl.Append( Float4, "Color" );

		{ // Create Vertex Buffer
			VertexBuffer::CreateInfo info;
			info.vtxCount = vertex_max_count;
			info.pVertexLayout = &vl;
			info.type = VertexBuffer::Type::Dynamic;
			m_pVertexBuffer = gfx->CreateVertexBuffer( info );
		}
		{ // Create Index Buffer
			IndexBuffer::CreateInfo info;
			info.idxCount = index_max_count;
			info.size = IndexBuffer::Size::UInt32;
			info.type = IndexBuffer::Type::Dynamic;
			m_pIndexBuffer = gfx->CreateIndexBuffer( info );
		}
		{ // Create Index Buffer
			ConstantBuffer::CreateInfo info;
			info.type = ConstantBuffer::Type::Dynamic;
			info.max_size = 128;
			m_pTransformBuffer = gfx->CreateConstantBuffer( info );

			const auto res = v2f32{ (float)_viewport_size.w, (float)_viewport_size.h };

			const auto screen = m44(
				2.0f / res.x,	0.0f,		   -1.0f, 0.0f,
				0.0f,		   -2.0f / res.y,	1.0f, 0.0f,
				0.0f,			0.0f,			1.0f, 0.0f,
				0.0f,			0.0f,			0.0f, 1.0f
			).transpose();

			m_pTransformBuffer->SetData( &screen, sizeof(screen) );
		}
		{ // Create Shaders
			Shader::CreateInfo info;
			info.pVertexLayout = &vl;
			info.sourceCode = font_emoji_shader;
			m_pShader = gfx->CreateShader( info );
			
			info.sourceCode = msdf_shader;
			m_pSDF_Shader = gfx->CreateShader( info );
		}
		{
			Blender::CreateInfo info;
			info.blend = Blender::Blend::Normal;
			m_pBlender = gfx->CreateBlender( info );
		}
		{
			Sampler::CreateInfo info;
			info.filter = Sampler::Linear;
			m_pSampler = gfx->CreateSampler( info );
		}
	}

	void TextRendererImpl::Destroy()
	{
		_aligned_free( f_DrawData.pVtxData );
		_aligned_free( f_DrawData.pIdxData );
		delete this;
	}

	void TextRendererImpl::OnResize( Graphics * , size2 size )
	{
		const auto screen = m44(
			2.0f / (float)size.w, 0.0f,                -1.0f, 0.0f,
			0.0f,                -2.0f / (float)size.h, 1.0f, 0.0f,
			0.0f,                 0.0f,                 1.0f, 0.0f,
			0.0f,                 0.0f,                 0.0f, 1.0f
		).transpose();

		m_pTransformBuffer->SetData( &screen, sizeof( screen ) );
	}

	void TextRendererImpl::render()
	{
		m_pVertexBuffer->SetData(f_DrawData.pVtxData, f_DrawData.vtxCount);
		m_pIndexBuffer ->SetData(f_DrawData.pIdxData, f_DrawData.idxCount);

		auto shader = m_pShader.get();
		if( m_Type == FontType::SDF ) shader = m_pSDF_Shader.get();

		shader->Bind();
		m_pVertexBuffer->Bind();
		m_pIndexBuffer->Bind();
		m_pTransformBuffer->Bind(gfx::ConstantBuffer::Target::Vertex, 0);
		m_pBlender->Bind();
		m_pSampler->Bind(gfx::Sampler::Pixel, 0);
		f_DrawData.Texture->Bind(0);

		m_pGraphics->DrawIndexed( f_DrawData.idxCount );

		f_DrawData.idxCount = 0;
		f_DrawData.vtxCount = 0;

		if( e_DrawData.idxCount == 0 )
			return;

		//! @Note: Better to just set all data, all at once?
		m_pVertexBuffer->SetData( e_DrawData.pVtxData, e_DrawData.vtxCount );
		m_pIndexBuffer ->SetData( e_DrawData.pIdxData, e_DrawData.idxCount );

		m_pShader->Bind();
		m_pVertexBuffer->Bind();
		m_pIndexBuffer->Bind();
		e_DrawData.Texture->Bind( 0 );

		m_pGraphics->DrawIndexed( e_DrawData.idxCount );

		e_DrawData.idxCount = 0;
		e_DrawData.vtxCount = 0;
	}


	v2f32 TextRendererImpl::text_bounds( struct Font* font, const char* text ) { return {}; }
	v2f32 TextRendererImpl::text_bounds_ui( struct UIFont* font, const chr* text ) { return {}; }
	v2f32 TextRendererImpl::text_bounds_sdf( struct SDFFont* font, const char* text, float size ) { return {}; }
	v2f32 TextRendererImpl::add_text( struct Font* font, const char* text, v2f32 pos, color color ) { return {}; }

	v2f32 TextRendererImpl::add_text_ui( struct UIFont* font, const chr* text, v2f32 pos, float size, color color )
	{
		m_Type = UI;
		f_DrawData.Texture = font->get_atlas();
		e_DrawData.Texture = font->get_emoji_atlas();

		const float start = pos.x;
		const Font::Glyph* glyph;

		for( chr cp = *text; cp != U'\0'; cp = *++text )
		{
			if( cp == U'\r' || cp == U'\n' )
			{
				pos.y += font->height() * size;
				pos.x = start;
				continue;
			}
			glyph = font->lookup( cp );

			if( glyph == font->fallback() ) {
				int char_advance;
				if( auto g = font->lookup_emoji( &cp, char_advance ) )
				{
					e_DrawData.add_glyph( g, pos, size, color );
					pos.x += g->advance * size;
					text += char_advance - 1;
					continue;
				}
			}

			if( cp != U' ' ) {
				f_DrawData.add_glyph( glyph, pos, size, color );
			}

			pos.x += glyph->advance * size;
		}

		return {};
	}

	v2f32 TextRendererImpl::add_text_sdf( struct SDFFont* font, const char* text, v2f32 pos, float size, color color )
	{
		m_Type = SDF;
		f_DrawData.Texture = font->get_atlas();

		const float start = pos.x;
		const float starty = pos.y;
		float width = 0.0f;
		const Font::Glyph* glyph;

		for( char c = *text; c != '\0'; c = *++text )
		{
			// newline
			if( c == '\r' || c == '\n' ) {
				auto w = pos.x - start;
				if( w > width )
					width = w;

				pos.y += size;
				pos.x = start;
				continue;
			}
			
			glyph = font->lookup( c );

			if( c != ' ' ) {
				f_DrawData.add_glyph( glyph, pos, size, color );
			}

			pos.x += glyph->advance * size;
		}

		{
			auto w = pos.x - start;
			if( width == 0.0f )
				width = w;
		}

		return { width, pos.y - starty + size };
	}



	void TextRendererImpl::DrawData::add_glyph( const Font::Glyph* g, const v2f32& origin, const float& scale, const color& color )
	{
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 1u;
		pIdxData[idxCount++] = vtxCount + 2u;
		pIdxData[idxCount++] = vtxCount + 3u;
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 2u;

		const auto rect = rf32{
			origin.x + scale * g->rc.x.low,
			origin.x + scale * g->rc.x.high,
			origin.y + scale * g->rc.y.low,
			origin.y + scale * g->rc.y.high,
		};

		pVtxData[vtxCount++] = vertex( v2f32( rect.x.low,  rect.y.high ), v2f32( g->uv.x.low,  g->uv.y.high ), color );
		pVtxData[vtxCount++] = vertex( v2f32( rect.x.low,  rect.y.low  ), v2f32( g->uv.x.low,  g->uv.y.low  ), color );
		pVtxData[vtxCount++] = vertex( v2f32( rect.x.high, rect.y.low  ), v2f32( g->uv.x.high, g->uv.y.low  ), color );
		pVtxData[vtxCount++] = vertex( v2f32( rect.x.high, rect.y.high ), v2f32( g->uv.x.high, g->uv.y.high ), color );
	}
}