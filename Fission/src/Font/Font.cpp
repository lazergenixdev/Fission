#include "Font.h"
#include <Fission/Base/Exception.hpp>
#include <Fission/Core/Engine.hh>
#include <MaxRectsBinPack.hpp>
#include <lunasvg.h>

#define _REPEAT_5 X(0) X(1) X(2) X(3) X(4)

namespace Fission {

	Font* Font::Create(const CreateInfo& info) {
		return new FontImpl(info);
	}

	UIFont* UIFont::Create(const CreateInfo& info) {
		return new UIFontImpl(info);
	}

}

namespace Fission {

	int CodepointCombinationCollection::match( const chr* codepoints ) const {
		// We only matched one, and that's ok :)
		if( second == NULL ) {
			return 1;
		}

		// you suck
		if( *codepoints++ != second )
			return 0;

		int n = 2;
		for( const auto& cp : extra ) {
			// is null, cannot possibly be equal to this codepoint
			if( *codepoints == NULL || *codepoints++ != cp )
				return 0;

			++n;
		}

		return n;
	}

	void EmojiDictionary::insert( const chr* codepoints, const Font::Glyph& glyph )
	{
		CodepointCombinationCollection codepoint_combination_collection;
		codepoint_combination_collection.glyph = glyph;
		codepoint_combination_collection.second = codepoints[1];

		if( codepoints[1] != 0 ) {
			for( int i = 2; codepoints[i] != 0; ++i ) {
				codepoint_combination_collection.extra.emplace_back( codepoints[i] );
			}
		}

		auto it = Map.find( *codepoints );
		if( it != Map.end() ) {
			it->second.emplace_back( std::move(codepoint_combination_collection) );
		} else {
			auto& entry = Map[*codepoints];
			entry.emplace_back( std::move( codepoint_combination_collection ) );
		}
	}

	inline void generate_font_atlas(
		FT_Face   face,
		u32       height,

		float&                                 outHeight,
		std::unordered_map<chr, Font::Glyph>&  outMap,
		fsn_ptr<gfx::Texture2D>&        outTexture,
		Font::Glyph&                           outFallback
	) {
		auto gfx = GetEngine()->GetGraphics();
		FT_Error error = FT_Err_Ok;

		if( error = FT_Set_Pixel_Sizes( face, 0, height ) )
			throw std::logic_error( "failed to set char size" );

		float yMax = float( face->size->metrics.ascender >> 6 );
		outHeight = float( face->size->metrics.height >> 6 );

		Surface::CreateInfo surf_info;
		surf_info.size = { 8 * (int)outHeight, 8 * (int)outHeight };
		//	surf_info.fillColor = color{};
		auto pSurface = Surface::Create( surf_info );

		auto pack = rbp::MaxRectsBinPack( surf_info.size.w, surf_info.size.h, false );

		auto generate_glyph = [&]() -> Font::Glyph {
			Font::Glyph g;

			g.offset.x = (float)( face->glyph->bitmap_left );
			g.offset.y = (float)( -face->glyph->bitmap_top ) + yMax;
			g.size.x = (float)( face->glyph->metrics.width >> 6 );
			g.size.y = (float)( face->glyph->metrics.height >> 6 );
			g.advance = (float)( face->glyph->metrics.horiAdvance >> 6 );

			auto bitmap = face->glyph->bitmap;

			auto rect = pack.Insert( bitmap.width + 1, bitmap.rows + 1, rbp::MaxRectsBinPack::RectBestAreaFit );
			//if( !node ) throw std::logic_error( "font atlas too small" );

			/* now, draw to our target surface */
			pSurface->insert(
				rect.x, rect.y,
				[&]( int x, int y ) -> color {
					return rgba8( 255, 255, 255, bitmap.buffer[y * bitmap.width + x] );
				},
				size2{ (int)bitmap.width, (int)bitmap.rows }
				);

			g.rc = {
				(float)rect.x / (float)pSurface->width(),
				(float)( rect.x + bitmap.width ) / (float)pSurface->width(),
				(float)rect.y / (float)pSurface->height(),
				(float)( rect.y + bitmap.rows ) / (float)pSurface->height()
			};

			return g;
		};

		if( error = FT_Load_Glyph( face, 0, FT_LOAD_RENDER ) )
			throw 0x45;

		// Set Fallback glyph
		outFallback = generate_glyph();

		// Glyph ranges for most english and latin, Todo: better font control (languages)
		for( chr ch = 0x20; ch < 0x7F; ch++ )
		{
			if( error = FT_Load_Char( face, ch, FT_LOAD_RENDER ) )
				continue;

			if( face->glyph->glyph_index == 0 ) // <-- why?
				continue;

			outMap.insert( std::make_pair( ch, generate_glyph() ) );
		}

		gfx::Texture2D::CreateInfo tex_info;
		tex_info.pSurface = pSurface.get();
		outTexture = gfx->CreateTexture2D( tex_info );
	}

	inline void generate_emoji_atlas(
		const void* data,
		u32         height,

		EmojiDictionary& outDict,
		fsn_ptr<gfx::Texture2D>& outTexture
	) {
		height = height & 0b11111111111111111110;
		auto gfx = GetEngine()->GetGraphics();

		struct EmojiData {
			chr codepoints[11];
			uint32_t offset;
		};

		u32 n = ((const u32*)data)[0];
		EmojiData* emoji_meta = (EmojiData*)( (const u32*)data + 1 );
		const char* svg_data = (const char*)data;

		outDict.Map.reserve(777); // jackpot

		Surface::CreateInfo surf_info;
		surf_info.size = { 61 * ((int)height+1), 61 * ((int)height+1) };
		auto pSurface = Surface::Create( surf_info );

		auto pack = rbp::MaxRectsBinPack( surf_info.size.w, surf_info.size.h, false );

		Font::Glyph g;
		g.offset.x = 0.0f;
		g.offset.y = 0.0f;
		g.size.x  = (float)height;
		g.size.y  = (float)height;
		g.advance = (float)height;

		auto bm = lunasvg::Bitmap( height, height );
		auto matrix = lunasvg::Matrix((float)height / 36.0f, 0, 0, (float)height / 36.0f, 0, 0);

		for( i64 i : rangei64(n) ) {
			u32 offset = emoji_meta[i].offset;
			chr cp     = emoji_meta[i].codepoints[0];

			// This is a private use codepoint, sorry Japan :,(
			if( cp == U'\uE50A' ) [[unlikely]] continue;

			auto w = height, h = height;
			auto rect = pack.Insert( w + 1, h + 1, rbp::MaxRectsBinPack::RectBottomLeftRule );

			auto doc = lunasvg::Document::loadFromData( svg_data + offset );
			memset( bm.data(), 0, height* height * sizeof rgba8 );
			doc->render( bm, matrix );
			const rgba8* data = (const rgba8*)bm.data();

			/* now, draw to our target surface */
			pSurface->insert(
				rect.x, rect.y,
				[&]( int x, int y ) -> color {
					auto& c = data[y * w + x];
					return rgba8( c.b, c.g, c.r, c.a );
				},
				size2{ (int)w, (int)h }
			);

			g.rc = {
				(float) rect.x            / (float)pSurface->width(),
				(float)(rect.x + height ) / (float)pSurface->width(),
				(float) rect.y            / (float)pSurface->height(),
				(float)(rect.y + height ) / (float)pSurface->height()
			};

			outDict.insert( emoji_meta[i].codepoints, g );
		}

		gfx::Texture2D::CreateInfo tex_info;
		tex_info.pSurface = pSurface.get();
		outTexture = gfx->CreateTexture2D( tex_info );
	}


	FontImpl::FontImpl( const CreateInfo& info )
		: m_Face(info.fontfile, info.fontfilesize), m_Height(info.size)
	{
		generate_font_atlas( m_Face.m_Face, (u32)info.size, m_Height, m_Map, m_pAtlasTexture, m_FallbackGlyph );
	}

	gfx::Texture2D* FontImpl::get_atlas() const {
		return m_pAtlasTexture.get();
	}

	const Font::Glyph* FontImpl::lookup( chr _Codepoint ) const {
		auto it = m_Map.find(_Codepoint);

		if( it != m_Map.end() )
			return &it->second;

		return &m_FallbackGlyph;
	}

	float FontImpl::height() const {
		return m_Height;
	}

	void FontImpl::resize( float _New_Size ) {
		FISSION_THROW_NOT_IMPLEMENTED();
	}

	void FontImpl::Destroy()
	{
		delete this;
	}


	UIFontImpl::UIFontImpl( const CreateInfo& info ):
		m_Face( info.fontfile, info.fontfilesize ),
		m_Size( info.size )
	{
		generate_font_atlas( m_Face.m_Face, (u32)info.size, m_Height, m_Map, m_pAtlasTexture, m_FallbackGlyph );
		generate_emoji_atlas( info.emojifile, (u32)m_Height, m_EmojiMap, m_pEmojiTexture );
	}

	gfx::Texture2D* UIFontImpl::get_atlas() const {
		return m_pAtlasTexture.get();
	}

	const Font::Glyph* UIFontImpl::lookup( chr _Codepoint ) const {
		auto it = m_Map.find( _Codepoint );

		if( it != m_Map.end() )
			return &it->second;

		return &m_FallbackGlyph;
	}

	float UIFontImpl::height() const {
		return m_Height;
	}

	void UIFontImpl::resize( float _New_Size ) {
	}

	void UIFontImpl::Destroy()
	{
		delete this;
	}

	float UIFontImpl::size() const {
		return m_Size;
	}

	const Font::Glyph* UIFontImpl::fallback() const {
		return &m_FallbackGlyph;
	}

	std::optional<const Font::Glyph*> UIFontImpl::lookup_emoji( const chr* codepoints, int& advance ) const
	{
		auto it = m_EmojiMap.Map.find( *codepoints++ );

		if( it != m_EmojiMap.Map.end() ) {
			const auto& codepoint_combination_collection_collection = it->second;

			const Glyph* winner = nullptr;
			int max = 0;

			for( const auto& a : codepoint_combination_collection_collection ) {
				int n = a.match(codepoints);
				if( n > max ) {
					max = n;
					winner = &a.glyph;
				}
			}

			advance = max;
			return winner != nullptr ? winner : (std::optional<const Font::Glyph*>());
		}

		return {};
	}

	gfx::Texture2D* UIFontImpl::get_emoji_atlas() const
	{
		return m_pEmojiTexture.get();
	}
}
