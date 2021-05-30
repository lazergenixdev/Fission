#include "Renderer2DImpl.h"
#include "Mesh.h"
#include <numbers>
#include <Fission/Base/Exception.h>

namespace Fission {

	std::unique_ptr<Renderer2D> Renderer2D::Create( Graphics * pGraphics ) {
		return std::make_unique<Renderer2DImpl>( pGraphics );
	}

	std::vector<Renderer2DImpl::DrawData::sincos> Renderer2DImpl::DrawData::TrigCache = [] () {
		using sincos = Renderer2DImpl::DrawData::sincos;
		std::vector<sincos> out;

		int geometry_persision = 10;

		assert( geometry_persision >= 1 && geometry_persision < 100 ); // restrict the amount of persision to a reasonable range

		out.reserve( geometry_persision );
		static constexpr float quarter_rotation = ( std::numbers::pi_v<float> / 2.0f );
		const float n = (float)( geometry_persision + 1 );

		// Calculate sin/cos to cache for later render commands
		for( int i = 0; i < geometry_persision; i++ ) {
			float x = (float)( i + 1 ) * quarter_rotation / n;
			out.emplace_back( sinf( x ), cosf( x ) );
		}

		return out;
	}();


	static constexpr const char * _fission_renderer2d_shader_code_hlsl =
		R"( 
	matrix screen;

struct VS_OUT { 
	float2 tc : TexCoord; 
	float4 color : Color; 
	float4 pos : SV_Position; 
};
VS_OUT vs_main( float2 pos : Position, float2 tc : TexCoord, float4 color : Color ) { 
	VS_OUT vso; 
	vso.pos = mul( float4( pos, 1.0, 1.0 ), screen ); 
	vso.tc = tc; 
	vso.color = color;
	return vso; 
}
Texture2D tex;
SamplerState ss;
float4 ps_main( float2 tc : TexCoord, float4 color : Color ) : SV_Target { 
	if( tc.x < -0.5f ) return color;
	return tex.Sample( ss, tc ) * color;
}
	)";

	Renderer2DImpl::Renderer2DImpl( Graphics * pGraphics )
		: m_pGraphics( pGraphics )
	{
		// Allocate aligned memory for faster access
		vertex_data = (vertex *)_aligned_malloc( vertex_max_count * sizeof vertex, 32 );
		index_data = (uint32_t *)_aligned_malloc( index_max_count * sizeof uint32_t, 32 );

		using namespace Resource;
		using namespace Resource::VertexLayoutTypes;

		auto vl = VertexLayout{};
		vl.Append<Float2>( "Position" );
		vl.Append<Float2>( "TexCoord" );
		vl.Append<Float4>( "Color" );

		{ // Create Vertex Buffer
			VertexBuffer::CreateInfo info;
			info.vtxCount = vertex_max_count;
			info.pVertexLayout = &vl;
			info.type = VertexBuffer::Type::Dynamic;
			m_pVertexBuffer = pGraphics->CreateVertexBuffer( info );
		}
		{ // Create Index Buffer
			IndexBuffer::CreateInfo info;
			info.idxCount = index_max_count;
			info.size = IndexBuffer::Size::UInt32;
			info.type = IndexBuffer::Type::Dynamic;
			m_pIndexBuffer = pGraphics->CreateIndexBuffer( info );
		}
		{ // Create Shaders
			Shader::CreateInfo info;
			info.pVertexLayout = &vl;
			if( pGraphics->GetAPI() == Graphics::API::DirectX11 )
				info.source_code = _fission_renderer2d_shader_code_hlsl;
			m_pShader = pGraphics->CreateShader( info );

			vec2f res = vec2f( 1280.0f, 720.0f ); // todo: this is a bug, please fix as soon as possible
			//vec2f res = vec2f( 1920.0f, 1080.0f ); // todo: this is a bug, please fix as soon as possible

			float matrix[16] = {
				2.0f / res.x,	0.0f,		   -1.0f, 0.0f,
				0.0f,		   -2.0f / res.y,	1.0f, 0.0f,
				0.0f,			0.0f,			1.0f, 0.0f,
				0.0f,			0.0f,			0.0f, 1.0f,
			};

			m_pShader->SetVariable( "screen", *(Shader::mat4x4f *)matrix );
		}
		{ // todo: more blenders
			Blender::CreateInfo info;

			info.blend = Blender::Blend::Normal;
			m_pBlenders[(int)BlendMode::Normal] = pGraphics->CreateBlender( info );

			info.blend = Blender::Blend::Add;
			m_pBlenders[(int)BlendMode::Add] = pGraphics->CreateBlender( info );

			m_pUseBlender = m_pBlenders[(int)BlendMode::Normal].get();
		}

		m_DrawBuffer.reserve( 20 );
		m_DrawBuffer.emplace_back( this, 0u, 0u );
	}

	Renderer2DImpl::~Renderer2DImpl() noexcept
	{
		_aligned_free( vertex_data );
		_aligned_free( index_data );
	}

	void Renderer2DImpl::Render()
	{
		if( m_DrawBuffer.size() == 1u && m_DrawBuffer[0].vtxCount == 0u ) return;

		auto & end = m_DrawBuffer.back();

		m_pShader->Bind();
		m_pVertexBuffer->SetData( vertex_data, end.vtxCount + end.vtxStart );
		m_pVertexBuffer->Bind();
		m_pIndexBuffer->SetData( index_data, end.idxCount + end.idxStart );
		m_pIndexBuffer->Bind();
		m_pUseBlender->Bind();

		for( auto && cmd : m_DrawBuffer )
		{
			if( cmd.Texture ) cmd.Texture->Bind(0);
			m_pGraphics->DrawIndexed( cmd.idxCount, cmd.idxStart, cmd.vtxStart );
		}

		m_DrawBuffer.clear();
		m_DrawBuffer.emplace_back( this, 0u, 0u );

	}

	void Renderer2DImpl::FillTriangle( base::vector2f p0, base::vector2f p1, base::vector2f p2, color color )
	{
		m_DrawBuffer.back().AddTriangle( p0, p1, p2, color, color, color );
	}

	void Renderer2DImpl::FillTriangleGrad( base::vector2f p0, base::vector2f p1, base::vector2f p2, color c0, color c1, color c2 )
	{
		m_DrawBuffer.back().AddTriangle( p0, p1, p2, c0, c1, c2 );
	}

	void Renderer2DImpl::FillTriangleUV( base::vector2f p0, base::vector2f p1, base::vector2f p2, base::vector2f uv0, base::vector2f uv1, base::vector2f uv2, Resource::Texture2D * pTexture, color tint )
	{
		SetTexture( pTexture );
		m_DrawBuffer.back().AddTriangleUV( p0, p1, p2, uv0, uv1, uv2, tint );
	}

	void Renderer2DImpl::FillRect( rectf rect, color color )
	{
		m_DrawBuffer.back().AddRectFilled( rect, color );
	}

	void Renderer2DImpl::DrawRect( rectf rect, color color, float stroke_width, StrokeStyle stroke )
	{
		m_DrawBuffer.back().AddRect( rect, color, stroke_width, stroke );
	}

	void Renderer2DImpl::FillRectGrad( rectf rect, color color_topleft, color color_topright, color color_bottomleft, color color_bottomright )
	{
		m_DrawBuffer.back().AddRectFilled( rect, color_topleft, color_topright, color_bottomleft, color_bottomright );
	}

	void Renderer2DImpl::FillRoundRect( rectf rect, float rad, color color )
	{
		m_DrawBuffer.back().AddRoundRectFilled( rect, rad, color );
	}

	void Renderer2DImpl::DrawRoundRect( rectf rect, float rad, color color, float stroke_width, StrokeStyle stroke )
	{
		m_DrawBuffer.back().AddRoundRect( rect, rad, color, stroke_width, stroke );
	}

	void Renderer2DImpl::DrawLine( base::vector2f start, base::vector2f end, color color, float stroke_width, StrokeStyle stroke )
	{
		(void)stroke;
		m_DrawBuffer.back().AddLine( start, end, stroke_width, color, color );
	}

	void Renderer2DImpl::FillCircle( base::vector2f point, float radius, color color )
	{
		m_DrawBuffer.back().AddCircleFilled( point, radius, color );
	}

	void Renderer2DImpl::DrawCircle( base::vector2f point, float radius, color color, float stroke_width, StrokeStyle stroke )
	{
		m_DrawBuffer.back().AddCircle( point, radius, color, color, stroke_width, stroke );
	}

	void Renderer2DImpl::DrawCircle( base::vector2f point, float radius, color inner_color, color outer_color, float stroke_width, StrokeStyle stroke )
	{
		m_DrawBuffer.back().AddCircle( point, radius, inner_color, outer_color, stroke_width, stroke );
	}

	void Renderer2DImpl::FillArrow( base::vector2f start, base::vector2f end, float width, color c )
	{
		if( start == end )
		{
			m_DrawBuffer.back().AddCircleFilled( start, width * 0.2f, c );
			return;
		}

		base::vector2f diff = start - end;
		base::vector2f par = diff.norm();
		float lensq = diff.lensq();

		if( lensq < width * width )
		{
			const base::vector2f center = start;
			const base::vector2f perp = par.perp() * ( sqrtf( lensq ) * 0.5f );
			const base::vector2f l = center - perp, r = center + perp;

			m_DrawBuffer.back().AddTriangle( end, l, r, c, c, c );
		}
		else
		{
			const base::vector2f perp = par.perp() * ( width * 0.5f );
			const base::vector2f center = end + par * width;
			const base::vector2f l = center - perp, r = center + perp;

			m_DrawBuffer.back().AddTriangle( end, l, r, c, c, c );
			m_DrawBuffer.back().AddLine( start, center, 0.4f * width, c, c );
		}

	}

	void Renderer2DImpl::DrawImage( Resource::Texture2D * pTexture, rectf rect, rectf uv, color tint )
	{
		SetTexture( pTexture );
		m_DrawBuffer.back().AddRectFilledUV( rect, uv, tint );
	}

	void Renderer2DImpl::DrawImage( Resource::Texture2D * pTexture, rectf rect, color tint )
	{
		SetTexture( pTexture );
		m_DrawBuffer.back().AddRectFilledUV( rect, { 0.0f, 1.0f, 0.0f, 1.0f }, tint );
	}

	void Renderer2DImpl::DrawMesh( const Mesh * m )
	{
		m_DrawBuffer.back().AddMesh( m );
	}

	void Renderer2DImpl::SelectFont( const Font * pFont )
	{
		m_pSelectedFont = pFont;
	}

	TextLayout Renderer2DImpl::DrawString( const wchar_t * wstr, base::vector2f pos, color c )
	{
		FISSION_ASSERT( m_pSelectedFont, "you're not supposed to do that." );

		SetTexture( m_pSelectedFont->GetTexture2D() );

		float start = 0.0f;
		const Font::Glyph * glyph;

		while( wstr[0] != L'\0' )
		{
			if( *wstr == L'\r' || *wstr == L'\n' ) { wstr++; pos.y += m_pSelectedFont->GetSize(); start = 0.0f; continue; }
			glyph = m_pSelectedFont->GetGylph( *wstr );


#define __SNAP_TEXT 1

#define _ROUND(_VAL) ((float)(int)((_VAL) + 0.5f))

#if __SNAP_TEXT
			const auto left = _ROUND( pos.x + glyph->offset.x + start );
			const auto right = _ROUND( left + glyph->size.x );
			const auto top = _ROUND( pos.y + glyph->offset.y );
			const auto bottom = _ROUND( top + glyph->size.y );
#else
			const auto left = pos.x + glyph->offset.x + start;
			const auto right = left + glyph->size.x;
			const auto top = pos.y + glyph->offset.y;
			const auto bottom = top + glyph->size.y;
#endif

			m_DrawBuffer.back().AddRectFilledUV( { left, right, top, bottom }, glyph->rc, c );

			start += glyph->advance;

			wstr++;
		}

		return TextLayout{ start, (float)m_pSelectedFont->GetSize() };
	}

	TextLayout Renderer2DImpl::CreateTextLayout( const wchar_t * wstr )
	{
		FISSION_ASSERT( m_pSelectedFont, "you're not supposed to do that." );

		TextLayout out{ 0.0f,(float)m_pSelectedFont->GetSize() };

		while( wstr[0] != L'\0' )
		{
			const Font::Glyph * g = m_pSelectedFont->GetGylph( *wstr );
		//	out.ch_divisions.emplace_back( out.width + g->advance * 0.5f, out.width );
			out.width += g->advance;
			wstr++;
		}

		return out;
	}

	TextLayout Renderer2DImpl::DrawString( const char * str, base::vector2f pos, color c )
	{
		FISSION_ASSERT( m_pSelectedFont, "you're not supposed to do that." );

		SetTexture( m_pSelectedFont->GetTexture2D() );

		float start = 0.0f;
		const Font::Glyph * glyph;

		while( str[0] != '\0' )
		{
			if( *str == '\r' || *str == '\n' ) { str++; pos.y += m_pSelectedFont->GetSize(); start = 0.0f; continue; }
			glyph = m_pSelectedFont->GetGylph( (wchar_t)*str );


#define __SNAP_TEXT 1

#define _ROUND(_VAL) ((float)(int)((_VAL) + 0.5f))

#if __SNAP_TEXT
			const auto left = _ROUND( pos.x + glyph->offset.x + start );
			const auto right = _ROUND( left + glyph->size.x );
			const auto top = _ROUND( pos.y + glyph->offset.y );
			const auto bottom = _ROUND( top + glyph->size.y );
#else
			const auto left = pos.x + glyph->offset.x + start;
			const auto right = left + glyph->size.x;
			const auto top = pos.y + glyph->offset.y;
			const auto bottom = top + glyph->size.y;
#endif

			m_DrawBuffer.back().AddRectFilledUV( { left, right, top, bottom }, glyph->rc, c );

			start += glyph->advance;

			str++;
		}

		return TextLayout{ start, (float)m_pSelectedFont->GetSize() };
	}

	TextLayout Renderer2DImpl::CreateTextLayout( const char * str )
	{
		FISSION_ASSERT( m_pSelectedFont, "you're not supposed to do that." );

		TextLayout out{ 0.0f,(float)m_pSelectedFont->GetSize() };

		while( str[0] != '\0' )
		{
			const Font::Glyph * g = m_pSelectedFont->GetGylph( (wchar_t)*str );
			out.width += g->advance;
			str++;
		}

		return out;
	}

	void Renderer2DImpl::SetBlendMode( BlendMode mode )
	{
		m_pUseBlender = m_pBlenders[(int)mode].get();
	}

	void Renderer2DImpl::PushTransform( const mat3x2f & transform )
	{
		m_TransformStack.emplace_back( transform );
		_set_accumulated_transform();
	}

	void Renderer2DImpl::PopTransform()
	{
		FISSION_ASSERT( !m_TransformStack.empty(), "No Transforms were left to be poped" );
		m_TransformStack.pop_back();
		_set_accumulated_transform();
	}

	void Renderer2DImpl::_set_accumulated_transform()
	{
		if( m_TransformStack.empty() ) {
			m_accTransform = mat3x2f::Identity();
			return;
		}

		m_accTransform = m_TransformStack.front();

		for( int i = 1; i < m_TransformStack.size(); i++ )
			m_accTransform = m_TransformStack[i] * m_accTransform;
	}

	void Renderer2DImpl::SetTexture( Resource::Texture2D * tex )
	{
		FISSION_ASSERT( tex, "bruh" );

		auto & end = m_DrawBuffer.back();
		if( end.Texture == nullptr ) end.Texture = tex;
		else if( end.Texture != tex )
		{
			m_DrawBuffer.emplace_back( this, end.vtxStart + end.vtxCount, end.idxStart + end.idxCount );
			m_DrawBuffer.back().Texture = tex;
		}
	}

	Renderer2DImpl::DrawData::DrawData( Renderer2DImpl * parent, uint32_t vc, uint32_t ic )
		: vtxStart( vc ), idxStart( ic ),
		pVtxData( parent->vertex_data + vtxStart ), pIdxData( parent->index_data + idxStart ),
		mat( &parent->m_accTransform )
	{}

	void Renderer2DImpl::DrawData::AddRect( rectf rect, color c, float stroke_width, StrokeStyle stroke )
	{
		for( int i = 0; i < 8; i++ ) {
		// I bet you've never seen code like this:
			i & 0x1 ? (
				pIdxData[idxCount++] = vtxCount + i,
				pIdxData[idxCount++] = vtxCount + ( i + 1u ) % 8u,
				pIdxData[idxCount++] = vtxCount + ( i + 2u ) % 8u
			) : (
				pIdxData[idxCount++] = vtxCount + i,
				pIdxData[idxCount++] = vtxCount + ( i + 2u ) % 8u,
				pIdxData[idxCount++] = vtxCount + ( i + 1u ) % 8u
			);
		}

		float in_l = rect.get_l();
		float in_r = rect.get_r();
		float in_t = rect.get_t();
		float in_b = rect.get_b();

		float out_l = rect.get_l();
		float out_r = rect.get_r();
		float out_t = rect.get_t();
		float out_b = rect.get_b();

		switch( stroke )
		{
		case StrokeStyle::Center:
		{
			float half_stroke = stroke_width / 2.0f;

			in_l += half_stroke, in_t += half_stroke;
			in_r -= half_stroke, in_b -= half_stroke;

			out_l -= half_stroke, out_t -= half_stroke;
			out_r += half_stroke, out_b += half_stroke;
			break;
		}
		case StrokeStyle::Inside:
		{
			stroke_width = std::min( stroke_width, ( out_b - out_t ) / 2.0f );
			stroke_width = std::min( stroke_width, ( out_r - out_l ) / 2.0f );

			in_l += stroke_width, in_t += stroke_width;
			in_r -= stroke_width, in_b -= stroke_width;
			break;
		}
		case StrokeStyle::Outside:
		{
			out_l -= stroke_width, out_t -= stroke_width;
			out_r += stroke_width, out_b += stroke_width;
			break;
		}
		default:throw std::logic_error( "this don't make no fucking sense" );
		}

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( out_l, out_b )), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( in_l, in_b )  ), c );

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( out_l, out_t )), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( in_l, in_t )  ), c );

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( out_r, out_t )), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( in_r, in_t )  ), c );

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( out_r, out_b )), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( in_r, in_b )  ), c );
	}

	void Renderer2DImpl::DrawData::AddRoundRectFilled( rectf rect, float rad, color c )
	{
		FISSION_THROW_NOT_IMPLEMENTED();
	}

	void Renderer2DImpl::DrawData::AddRoundRect( rectf rect, float rad, color color, float stroke_width, StrokeStyle stroke )
	{
		FISSION_THROW_NOT_IMPLEMENTED();
	}

	void Renderer2DImpl::DrawData::AddMesh( const Mesh * m )
	{
		const auto * colors = m->m_Data->color_buffer.data();

		for( auto && i : m->m_Data->index_buffer )
			pIdxData[idxCount++] = i + vtxCount;

		for( auto && v : m->m_Data->vertex_buffer )
			pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(v.pos)), colors[v.color_index] );
	}

	void Renderer2DImpl::DrawData::AddCircleFilled( base::vector2f center, float rad, color c )
	{
		const int count = ( (int)TrigCache.size() + 1u ) * 4u - 2;
		for( int i = 0; i < count; i++ )
		{
			pIdxData[idxCount++] = vtxCount;
			pIdxData[idxCount++] = vtxCount + i + 1u;
			pIdxData[idxCount++] = vtxCount + i + 2u;
		}

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x + rad, center.y )), c );
		for( auto && trig : TrigCache )
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x + trig.cos * rad, center.y + trig.sin * rad )), c );

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x, center.y + rad )), c );
		for( auto && trig : TrigCache )
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x - trig.sin * rad, center.y + trig.cos * rad )), c );

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x - rad, center.y )), c );
		for( auto && trig : TrigCache )
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x - trig.cos * rad, center.y - trig.sin * rad )), c );

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x, center.y - rad )), c );
		for( auto && trig : TrigCache )
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x + trig.sin * rad, center.y - trig.cos * rad )), c );
	}

	void Renderer2DImpl::DrawData::AddCircle( base::vector2f center, float rad, color inc, color outc, float stroke_width, StrokeStyle stroke )
	{
		const int count = ( (int)TrigCache.size() + 1u ) * 4u;
		float hw = stroke_width * 0.5f;
		float inner_rad, outer_rad;
		switch( stroke )
		{
		case Fission::StrokeStyle::Center: { float hsw = stroke_width * 0.5f; inner_rad= rad - hsw; outer_rad= rad + hsw; break; }
		case Fission::StrokeStyle::Inside:	inner_rad= rad - stroke_width, outer_rad= rad; break;
		case Fission::StrokeStyle::Outside: inner_rad= rad, outer_rad= rad + stroke_width; break;
		default: throw std::logic_error( "this don't make no fucking sense" );
		}

		// This algorithm could probably be a heck of a lot more optimized,
		// but honesty this is good enough and I don't feel like wasting any more time on this.

		int count2 = count * 2;
		for( int i = 0; i < count; i++ )
		{
			pIdxData[idxCount++] = vtxCount + (i * 2u	  )%count2;
			pIdxData[idxCount++] = vtxCount + (i * 2u + 1u)%count2;
			pIdxData[idxCount++] = vtxCount + (i * 2u + 2u)%count2;

			pIdxData[idxCount++] = vtxCount + (i * 2u + 1u)%count2;
			pIdxData[idxCount++] = vtxCount + (i * 2u + 3u)%count2;
			pIdxData[idxCount++] = vtxCount + (i * 2u + 2u)%count2;
		}

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x + inner_rad, center.y )), inc );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x + outer_rad, center.y )), outc );
		for( auto && trig : TrigCache )
			pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x + trig.cos * inner_rad, center.y + trig.sin * inner_rad )), inc ),
			pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x + trig.cos * outer_rad, center.y + trig.sin * outer_rad )), outc );

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x, center.y + inner_rad )), inc );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x, center.y + outer_rad )), outc );
		for( auto && trig : TrigCache )
			pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x - trig.sin * inner_rad, center.y + trig.cos * inner_rad )), inc ),
			pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x - trig.sin * outer_rad, center.y + trig.cos * outer_rad )), outc );

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x - inner_rad, center.y )), inc );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x - outer_rad, center.y )), outc );
		for( auto && trig : TrigCache )
			pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x - trig.cos * inner_rad, center.y - trig.sin * inner_rad )), inc ),
			pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x - trig.cos * outer_rad, center.y - trig.sin * outer_rad )), outc );

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x, center.y - inner_rad )), inc );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x, center.y - outer_rad )), outc );
		for( auto && trig : TrigCache )
			pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x + trig.sin * inner_rad, center.y - trig.cos * inner_rad )), inc ),
			pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( center.x + trig.sin * outer_rad, center.y - trig.cos * outer_rad )), outc );
	}

	void Renderer2DImpl::DrawData::AddTriangle( base::vector2f p0, base::vector2f p1, base::vector2f p2, color c0, color c1, color c2 )
	{
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 1u;
		pIdxData[idxCount++] = vtxCount + 2u;

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(p0)), c0 );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(p1)), c1 );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(p2)), c2 );
	}

	void Renderer2DImpl::DrawData::AddTriangleUV( base::vector2f p0, base::vector2f p1, base::vector2f p2, base::vector2f uv0, base::vector2f uv1, base::vector2f uv2, color c )
	{
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 1u;
		pIdxData[idxCount++] = vtxCount + 2u;

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(p0)), uv0, c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(p1)), uv1, c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(p2)), uv2, c );
	}

	void Renderer2DImpl::DrawData::AddLine( base::vector2f start, base::vector2f end, float stroke, color startColor, color endColor )
	{
		const auto edge_vector = ( end - start ).perp().norm() * stroke / 2.0f;

		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 1u;
		pIdxData[idxCount++] = vtxCount + 2u;
		pIdxData[idxCount++] = vtxCount + 2u;
		pIdxData[idxCount++] = vtxCount + 1u;
		pIdxData[idxCount++] = vtxCount + 3u;

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(start + edge_vector)), startColor );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(start - edge_vector)), startColor );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(end + edge_vector)), endColor );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(end - edge_vector)), endColor );
	}

	void Renderer2DImpl::DrawData::AddRectFilledUV( rectf rect, rectf uv, color c )
	{
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 1u;
		pIdxData[idxCount++] = vtxCount + 2u;
		pIdxData[idxCount++] = vtxCount + 3u;
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 2u;

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.low , rect.y.high )), base::vector2f( uv.x.low , uv.y.high ), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.low , rect.y.low  )), base::vector2f( uv.x.low , uv.y.low  ), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.high, rect.y.low  )), base::vector2f( uv.x.high, uv.y.low  ), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.high, rect.y.high )), base::vector2f( uv.x.high, uv.y.high ), c );
	}

	void Renderer2DImpl::DrawData::AddRectFilled( base::vector2f tl, base::vector2f tr, base::vector2f bl, base::vector2f br, color c )
	{
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 1u;
		pIdxData[idxCount++] = vtxCount + 2u;
		pIdxData[idxCount++] = vtxCount + 3u;
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 2u;

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(bl)), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(tl)), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(tr)), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2f::from(br)), c );
	}

	void Renderer2DImpl::DrawData::AddRectFilled( rectf rect, color c )
	{
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 1u;
		pIdxData[idxCount++] = vtxCount + 2u;
		pIdxData[idxCount++] = vtxCount + 3u;
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 2u;

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.low , rect.y.high )), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.low , rect.y.low  )), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.high, rect.y.low  )), c );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.high, rect.y.high )), c );
	}

	void Renderer2DImpl::DrawData::AddRectFilled( rectf rect, color tl, color tr, color bl, color br )
	{
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 1u;
		pIdxData[idxCount++] = vtxCount + 2u;
		pIdxData[idxCount++] = vtxCount + 3u;
		pIdxData[idxCount++] = vtxCount;
		pIdxData[idxCount++] = vtxCount + 2u;

		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.low , rect.y.high )), bl );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.low , rect.y.low  )), tl );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.high, rect.y.low  )), tr );
		pVtxData[vtxCount++] = vertex( base::vector2f::from(*mat * vec2( rect.x.high, rect.y.high )), br );
	}

}