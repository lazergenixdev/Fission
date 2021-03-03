#include "SurfaceImpl.h"
//#include "Platform/Windows/hr_Exception.h"
#include "lazer/unfinished.h"

namespace Fission {

	std::unique_ptr<Surface> Surface::Create( const CreateInfo & info ) {
		switch( info.Format )
		{
		case Texture::Format_RGBA8_UNORM:
			return std::make_unique<SurfaceRGBA8_UNormImpl>( info );
		default: _lazer_throw_not_implemented;
		}
	}

	SurfaceRGBA8_UNormImpl::SurfaceRGBA8_UNormImpl( const Surface::CreateInfo & info )
	{
		if( info.Width > 0u && info.Height > 0u )
		{
			m_pData = std::make_unique<coloru[]>( info.Width * info.Height );
			m_Width = info.Width;
			m_Height = info.Height;
		}
	}

	void SurfaceRGBA8_UNormImpl::resize( vec2 new_size, ResizeOptions_ options )
	{
		_lazer_throw_not_implemented;
	}

	void SurfaceRGBA8_UNormImpl::set_width( uInt32 new_width, ResizeOptions_ options )
	{
		_lazer_throw_not_implemented;
	}

	void SurfaceRGBA8_UNormImpl::set_height( uInt32 new_height, ResizeOptions_ options )
	{
		_lazer_throw_not_implemented;
	}

	void SurfaceRGBA8_UNormImpl::insert( uInt32 x, uInt32 y, PixelCallback src, vec2 src_size )
	{
		assert( x + src_size.x <= (int)m_Width );
		assert( y + src_size.y <= (int)m_Height );

		for( int py = 0; py < src_size.y; py++ )
		{
			int dst_y = py + y;
			coloru * copy_dest = &( (coloru *)this->m_pData.get() )[dst_y * this->m_Width + x];

			for( int x = 0; x < src_size.x; x++ )
				copy_dest[x] = src( x, py );
		}
	}

	void SurfaceRGBA8_UNormImpl::insert( uInt32 x, uInt32 y, const Surface * src, std::optional<recti> src_region )
	{

		_lazer_throw_not_implemented;
	}

	Texture::Format SurfaceRGBA8_UNormImpl::format() const
	{
		return Texture::Format_RGBA8_UNORM;
	}

	Fission::color SurfaceRGBA8_UNormImpl::GetPixel( uInt32 x, uInt32 y )
	{
		_lazer_throw_not_implemented;
	}

	void SurfaceRGBA8_UNormImpl::PutPixel( uInt32 x, uInt32 y, color color )
	{
		FISSION_ASSERT( x < m_Width, "X out of range" );
		FISSION_ASSERT( y < m_Height, "Y out of range" );

		m_pData[y * m_Width + x] = color;
	}

	void SurfaceRGBA8_UNormImpl::shrink_to_fit( color clear_color )
	{
	}

	const void * SurfaceRGBA8_UNormImpl::data() const
	{
		return m_pData.get();
	}

	void * SurfaceRGBA8_UNormImpl::data()
	{
		return m_pData.get();
	}

	uInt32 SurfaceRGBA8_UNormImpl::width() const
	{
		return m_Width;
	}

	uInt32 SurfaceRGBA8_UNormImpl::height() const
	{
		return m_Height;
	}

	Surface::vec2 SurfaceRGBA8_UNormImpl::size() const
	{
		return vec2( m_Width, m_Height );
	}

	uInt32 SurfaceRGBA8_UNormImpl::byte_size() const
	{
		return m_cbSize;
	}

	uInt32 SurfaceRGBA8_UNormImpl::pixel_count() const
	{
		return m_pxCount;
	}

	bool SurfaceRGBA8_UNormImpl::empty() const
	{
		return !m_pData;
	}

}
