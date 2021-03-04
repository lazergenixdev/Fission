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
		virtual bool Save( const file::path & _FilePath ) const override;
		virtual void resize( vec2u new_size, ResizeOptions_ options = ResizeOptions_Clip ) override;
		virtual void set_width( uint32_t new_width, ResizeOptions_ options = ResizeOptions_Clip ) override;
		virtual void set_height( uint32_t new_height, ResizeOptions_ options = ResizeOptions_Clip ) override;


		virtual void insert( uint32_t x, uint32_t y, PixelCallback src, vec2u src_size ) override;
		virtual void insert( uint32_t x, uint32_t y, const Surface * src, std::optional<recti> src_region ) override;

		virtual Texture::Format format() const override;
		virtual color GetPixel( uint32_t x, uint32_t y ) const override;
		virtual void PutPixel( uint32_t x, uint32_t y, color color ) override;
		virtual void shrink_to_fit( color clear_color = coloru( 0, 0, 0, 0 ) ) override;
		virtual const void * data() const override;
		virtual void * data() override;
		virtual uint32_t width() const override;
		virtual uint32_t height() const override;
		virtual vec2u size() const override;
		virtual uint32_t byte_size() const override;
		virtual uint32_t pixel_count() const override;
		virtual bool empty() const override;

	private:
		uint32_t m_Width = 0, m_Height = 0, 
				m_pxCount = 0, m_cbSize = 0;
		std::unique_ptr<coloru[]> m_pData;
	};

}
