#include <Fission/Core/Application.hh>
#include <Fission/Base/util/SmartPointer.hpp>
#include <Fission/Core/Graphics/Font.hh>
#include <Fission/Core/Surface.hh>
#include "freetype.h"
#include <unordered_map>

namespace Fission {

	// You be Unicode, You get bad type names, You deserve it.
	struct CodepointCombinationCollection {
		chr              second; // most codepoints will be either 1 or 2, so Optimization! Yay! ._.
		std::vector<chr> extra;
		Font::Glyph      glyph;

		int match( const chr* codepoints ) const;
	};

	struct EmojiDictionary {
		void insert( const chr* codepoints, const Font::Glyph& glyph );

		// Unicode has literal millions of different codepoints going unused, and this is how it chooses to treat emojis, just shameful...
		std::unordered_map<chr, std::vector<CodepointCombinationCollection>> Map;
	};

	struct FontImpl : public Font
	{
		fsn_ptr<gfx::Texture2D>	m_pAtlasTexture;
		std::unordered_map<chr, Glyph>	m_Map;
		Glyph							m_FallbackGlyph;
		float							m_Height;

		freetype::Face					m_Face;

		FontImpl( const CreateInfo& info );

		virtual gfx::Texture2D* get_atlas() const override;
		virtual const Glyph* lookup( chr _Codepoint ) const override;
		virtual float height() const override;
		virtual void resize( float _New_Size ) override;
		virtual void Destroy() override;
	};

	struct UIFontImpl : public UIFont
	{
		freetype::Face m_Face;

		float  m_Size;
		float  m_Height;
		Glyph  m_FallbackGlyph;

		fsn_ptr<gfx::Texture2D>	       m_pAtlasTexture;
		std::unordered_map<chr, Glyph> m_Map;

		EmojiDictionary				   m_EmojiMap;
		fsn_ptr<gfx::Texture2D>	       m_pEmojiTexture;


		UIFontImpl( const CreateInfo& info );
		
		virtual gfx::Texture2D* get_atlas() const override;
		virtual const Glyph* lookup( chr _Codepoint ) const override;
		virtual float height() const override;
		virtual void resize( float _New_Size ) override;
		virtual void Destroy() override;

		virtual float size() const override;
		virtual const Glyph* fallback() const override;

		virtual const Glyph* lookup_emoji( const chr* codepoints, int& advance ) const override;
		virtual gfx::Texture2D* get_emoji_atlas() const override;
	};

	struct SDFFontImpl : public SDFFont
	{
		fsn_ptr<gfx::Texture2D>	 m_pAtlasTexture;
		std::vector<Glyph>	     m_Map;
		Glyph*					 m_FallbackGlyph;
		float					 m_Height;

		SDFFontImpl( const CreateInfo& info );

		virtual gfx::Texture2D* get_atlas() const override;
		virtual const Glyph* lookup( chr _Codepoint ) const override;
		virtual float height() const override;
		virtual void resize( float _New_Size ) override;
		virtual void Destroy() override;
	};

}
