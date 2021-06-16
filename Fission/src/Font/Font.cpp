#include <Fission/Core/Application.hh>
#include <Fission/Base/Utility/SmartPointer.h>
#include <Fission/Core/Graphics/Font.hh>
#include <Fission/Core/Surface.hh>
#include "freetype.h"

namespace Fission {

	static std::map<std::string, std::unique_ptr<Font>> s_Fonts;

	struct Node
	{
		using texID = Surface *;

		Node() = default;
		bool is_leaf() const { return ( A == nullptr ); }
		Node * insert( int width, int height )
		{
			if( is_leaf() )
			{
				auto sz = rc.size();

				//( if there's already a lightmap here, return )
				if( isFilled ) return NULL;

				//( if we're too small, return )
				if( height > sz.h || width > sz.w )
					return NULL;

				//( if we're just right, accept )
				if( sz == base::size{ width, height } )
				{
					isFilled = true;
					return this;
				}

				//( otherwise, gotta split this node and create some kids )
				A = new Node;
				B = new Node;

				//( decide which way to split )
				auto dw = sz.w - width;
				auto dh = sz.h - height;

				if( dw > dh )
				{
					A->rc = base::recti( rc.left(), rc.left() + width, rc.top(), rc.bottom() );
					B->rc = base::recti( rc.left() + width, rc.right(), rc.top(), rc.bottom() );
				}
				else
				{
					A->rc = base::recti( rc.left(), rc.right(), rc.top(), rc.top() + height );
					B->rc = base::recti( rc.left(), rc.right(), rc.top() + height, rc.bottom() );
				}

				//( insert into first child we created )
				return A->insert( width, height );
			}
			else
			{
				//( try inserting into first child )
				auto newNode = A->insert( width, height );
				if( newNode != NULL ) return newNode;
				//( no room, insert into second )
				return B->insert( width, height );
			}
		}

		Node * A = nullptr, * B = nullptr;
		bool isFilled = false;
		base::recti rc;
	};

	// Bitmap Font
	class _bm_Font : public Font
	{
	public:

		virtual Resource::IFTexture2D * GetTexture2D() const override { return const_cast<Resource::IFTexture2D*>( m_pTexture ); }
		virtual float GetSize() const override { return height; }

		virtual const Glyph * GetGylph( wchar_t ch ) const override
		{
			if( ch >= m_GlyphLookup.size() )
				return pFallbackGlyph;

			auto i = m_GlyphLookup[ch];
			return &m_Glyphs[i];
		}
		virtual const Glyph * GetGylphOutline( wchar_t ch ) const override { return nullptr; }

		std::vector<int> m_GlyphLookup;
		std::vector<Glyph> m_Glyphs;
		Glyph * pFallbackGlyph;
		Resource::IFTexture2D * m_pTexture; // This is a memory leak, LOL
		float size, height;
	};

	Font * FontManager::GetFont( const char * key )
	{
		auto it = s_Fonts.find( key );

		if( it != s_Fonts.end() )
			return it->second.get();

		return nullptr;
	}

	void add_font( const char * key, freetype::Face * pFace, float size, IFGraphics * gfx )
	{
		FT_Error error = FT_Err_Ok;
		_bm_Font font;

		FT_Face & face = pFace->m_Face;

		int dpi = 64;

		if( error = FT_Set_Char_Size(
			face,					/* handle to face object           */
			0,						/* char_width in 1/64th of points  */
			(FT_F26Dot6)size * 64,	/* char_height in 1/64th of points */
			dpi,					/* horizontal device resolution    */
			dpi ) )					/* vertical device resolution      */
			return;

		float yMax = float( face->size->metrics.ascender >> 6 );
		font.height = float( face->size->metrics.height >> 6 );

		Surface::CreateInfo surf_info;
		surf_info.size = { 1024, 256 };
		surf_info.fillColor = color{0.0f};
		auto pSurface = Surface::Create( surf_info );

		Node root;
		root.rc = base::recti( 0, pSurface->width(), 0, pSurface->height() );

		auto save_glyph = [&] ( wchar_t ch ) {
			Font::Glyph g;

			g.offset.x = (float)( face->glyph->bitmap_left );
			g.offset.y = (float)( -face->glyph->bitmap_top ) + yMax;
			g.size.x = (float)( face->glyph->metrics.width >> 6 );
			g.size.y = (float)( face->glyph->metrics.height >> 6 );
			g.advance = (float)( face->glyph->metrics.horiAdvance >> 6 );

			auto bitmap = face->glyph->bitmap;

			auto node = root.insert( bitmap.width, bitmap.rows );
			if( !node ) throw std::logic_error( "font atlas too small" );

			/* now, draw to our target surface */
			pSurface->insert(
				node->rc.left(), node->rc.top(),
				[&] ( int x, int y ) -> color {
					return rgba_color8( 255, 255, 255, bitmap.buffer[y * bitmap.width + x] );
				},
				base::size{ (int)bitmap.width, (int)bitmap.rows }
			);

			g.rc = {
				(float)node->rc.left() / (float)pSurface->width(),
				(float)node->rc.right() / (float)pSurface->width(),
				(float)node->rc.top() / (float)pSurface->height(),
				(float)node->rc.bottom() / (float)pSurface->height()
			};

			font.m_GlyphLookup[ch] = (int)font.m_Glyphs.size();
			font.m_Glyphs.emplace_back( g );
		};

		if( error = FT_Load_Glyph( face, 0, FT_LOAD_RENDER ) )
			throw 0x45;

		font.m_GlyphLookup.resize( 0xFF );

		save_glyph( L'\0' );

		// Glyph ranges for most english and latin, Todo: better font control (languages, storing fonts)
		for( auto ch = 0x20; ch < 0x7F; ch++ )
		{
			if( error = FT_Load_Char( face, ch, FT_LOAD_RENDER ) )
				continue;

			if( face->glyph->glyph_index == 0 )
				continue;

			save_glyph( ch );
		}
		for( auto ch = 0xA0; ch < 0xFF; ch++ )
		{
			if( error = FT_Load_Char( face, ch, FT_LOAD_RENDER ) )
				continue;

			if( face->glyph->glyph_index == 0 )
				continue;

			save_glyph( ch );
		}

		Resource::IFTexture2D::CreateInfo tex_info;
		tex_info.pSurface = pSurface.get();
		font.m_pTexture = gfx->CreateTexture2D( tex_info );

		auto & pfont = s_Fonts[key];
		pfont = std::make_unique<_bm_Font>( std::move( font ) );
		((_bm_Font*)pfont.get())->pFallbackGlyph = &( (_bm_Font *)pfont.get() )->m_Glyphs.front();
	}

	void FontManager::SetFont( const char * key, const std::filesystem::path & filepath, float pxsize, IFGraphics * gfx )
	{
		auto pFace = freetype::Library::LoadFaceFromFile( filepath );

		add_font( key, pFace, pxsize, gfx );
	}

	void FontManager::SetFont( const char * key, const void * data, size_t size, float pxsize, IFGraphics * gfx )
	{
		auto pFace = freetype::Library::LoadFaceFromMemory( data, size );

		add_font( key, pFace, pxsize, gfx );
	}

	void FontManager::DelFont( const char * key )
	{
		s_Fonts.clear();
	}

}