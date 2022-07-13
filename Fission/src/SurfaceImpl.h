#pragma once
#include <Fission/Core/Surface.hh>

namespace Fission
{

	class SurfaceRGBA8_UNormImpl : public Surface
	{
	public:
		SurfaceRGBA8_UNormImpl( const Surface::CreateInfo & info );

		// Inherited via Surface
		virtual bool Load( const std::filesystem::path & _FilePath ) override;
		virtual bool Save( const std::filesystem::path & _FilePath ) const override;

		virtual void resize( size2 _New_Size, ResizeOptions_ _Options ) override;
		virtual void set_width( int _New_Width, ResizeOptions_ _Options ) override;
		virtual void set_height( int _New_Height, ResizeOptions_ _Options ) override;

		virtual void insert( int _X, int _Y, PixelCallback _Source, size2 _Source_Size ) override;
		virtual void insert( int _X, int _Y, const Surface * _Source, std::optional<ri32> _Source_Rect ) override;

		virtual void PutPixel( int _X, int _Y, color _Color ) override;
		virtual color GetPixel( int _X, int _Y ) const override;

		virtual void shrink_to_fit( color clear_color ) override;

		virtual Texture::Format format() const override;
		virtual const void * data() const override;
		virtual void * data() override;
		virtual int width() const override;
		virtual int height() const override;
		virtual size2 size() const override;
		virtual base::size_t byte_size() const override;
		virtual base::size_t pixel_count() const override;
		virtual bool empty() const override;

	private:
		uint32_t m_Width = 0,   m_Height = 0, 
		         m_pxCount = 0, m_cbSize = 0;
		std::unique_ptr<rgba_color8[]> m_pData;
	};

}
