#pragma once
#include "Fission/Core/Surface.h"

namespace Fission
{

	class SurfaceRGBA8_UNormImpl : public Surface
	{
	public:
		SurfaceRGBA8_UNormImpl( const Surface::CreateInfo & info );

		// Inherited via Surface
		virtual bool Load( const file::path & _FilePath ) override;
		virtual bool Save( const file::path & _FilePath ) override;
		virtual void resize( vec2 new_size, ResizeOptions_ options = ResizeOptions_Clip ) override;
		virtual void set_width( uInt32 new_width, ResizeOptions_ options = ResizeOptions_Clip ) override;
		virtual void set_height( uInt32 new_height, ResizeOptions_ options = ResizeOptions_Clip ) override;


		virtual void insert( uInt32 x, uInt32 y, PixelCallback src, vec2 src_size ) override;
		virtual void insert( uInt32 x, uInt32 y, const Surface * src, std::optional<recti> src_region ) override;

		virtual Texture::Format format() const override;
		virtual color GetPixel( uInt32 x, uInt32 y ) override;
		virtual void PutPixel( uInt32 x, uInt32 y, color color ) override;
		virtual void shrink_to_fit( color clear_color = coloru( 0, 0, 0, 0 ) ) override;
		virtual const void * data() const override;
		virtual void * data() override;
		virtual uInt32 width() const override;
		virtual uInt32 height() const override;
		virtual vec2 size() const override;
		virtual uInt32 byte_size() const override;
		virtual uInt32 pixel_count() const override;
		virtual bool empty() const override;

	private:
		uint32_t m_Width = 0, m_Height = 0, 
				m_pxCount = 0, m_cbSize = 0;
		std::unique_ptr<coloru[]> m_pData;
	};

}
