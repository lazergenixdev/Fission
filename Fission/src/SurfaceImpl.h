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

		virtual void resize( vec2i _New_Size, ResizeOptions_ _Options ) override;
		virtual void set_width( int _New_Width, ResizeOptions_ _Options ) override;
		virtual void set_height( int _New_Height, ResizeOptions_ _Options ) override;

		virtual void insert( int _X, int _Y, PixelCallback _Source, vec2u _Source_Size ) override;
		virtual void insert( int _X, int _Y, const Surface * _Source, std::optional<recti> _Source_Rect ) override;

		virtual void PutPixel( int _X, int _Y, color _Color ) override;
		virtual color GetPixel( int _X, int _Y ) const override;

		virtual void shrink_to_fit( color clear_color = coloru( 0, 0, 0, 0 ) ) override;

		virtual Texture::Format format() const override;
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
