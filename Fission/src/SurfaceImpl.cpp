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
		FISSION_ASSERT( info.Width < 16384u, "Texture Width too big!" );
		FISSION_ASSERT( info.Height < 16384u, "Texture Height too big!" );

		if( info.Width > 0u && info.Height > 0u )
		{
			// width * height should never overflow, but make sure it doesn't otherwise stupid compiler won't shutup
			m_pData = std::make_unique<coloru[]>( (size_t)info.Width * (size_t)info.Height );
			m_Width = info.Width;
			m_Height = info.Height;
			m_pxCount = m_Width * m_Height;
			m_cbSize = m_pxCount * sizeof( coloru );
			if( info.FillColor.has_value() )
			{
				coloru fill = (coloru)info.FillColor.value();
				for( size_t i = 0; i < m_pxCount; i++ ) m_pData[i] = fill;
			}
		}
	}

	void SurfaceRGBA8_UNormImpl::resize( vec2u new_size, ResizeOptions_ options )
	{
		_lazer_throw_not_implemented;
	}

	void SurfaceRGBA8_UNormImpl::set_width( uint32_t new_width, ResizeOptions_ options )
	{
		_lazer_throw_not_implemented;
	}

	void SurfaceRGBA8_UNormImpl::set_height( uint32_t new_height, ResizeOptions_ options )
	{
		_lazer_throw_not_implemented;
	}

	void SurfaceRGBA8_UNormImpl::insert( uint32_t x, uint32_t y, PixelCallback src, vec2u src_size )
	{
		assert( x + src_size.x <= m_Width );
		assert( y + src_size.y <= m_Height );

		for( uint32_t py = 0; py < src_size.y; py++ )
		{
			uint32_t dst_y = py + y;
			coloru * copy_dest = &( (coloru *)this->m_pData.get() )[dst_y * this->m_Width + x];

			for( uint32_t x = 0; x < src_size.x; x++ )
				copy_dest[x] = src( x, py );
		}
	}

	void SurfaceRGBA8_UNormImpl::insert( uint32_t x, uint32_t y, const Surface * src, std::optional<recti> src_region )
	{
		vec2u start, size;
		if( src_region.has_value() )
			size = (vec2u)src_region->size(), start = (vec2u)src_region->get_tl();
		else
			size = src->size();

		FISSION_ASSERT( x + size.x <= m_Width );
		FISSION_ASSERT( y + size.y <= m_Height );

		for( uint32_t py = start.y; py < size.y; py++ )
		{
			coloru * copy_dest = &this->m_pData[( py + y ) * this->m_Width + x];

			for( uint32_t x = 0; x < size.x; x++ )
				copy_dest[x] = (coloru)src->GetPixel( x + start.x, py );
		}
	}

	Texture::Format SurfaceRGBA8_UNormImpl::format() const
	{
		return Texture::Format_RGBA8_UNORM;
	}

	Fission::color SurfaceRGBA8_UNormImpl::GetPixel( uint32_t x, uint32_t y ) const
	{
		return (color)m_pData[y * m_Width + x];
	}

	void SurfaceRGBA8_UNormImpl::PutPixel( uint32_t x, uint32_t y, color color )
	{
		FISSION_ASSERT( x < m_Width, "X out of range" );
		FISSION_ASSERT( y < m_Height, "Y out of range" );

		m_pData[y * m_Width + x] = color;
	}

	void SurfaceRGBA8_UNormImpl::shrink_to_fit( color clear_color )
	{
		_lazer_throw_not_implemented;
	}

	const void * SurfaceRGBA8_UNormImpl::data() const
	{
		return m_pData.get();
	}

	void * SurfaceRGBA8_UNormImpl::data()
	{
		return m_pData.get();
	}

	uint32_t SurfaceRGBA8_UNormImpl::width() const
	{
		return m_Width;
	}

	uint32_t SurfaceRGBA8_UNormImpl::height() const
	{
		return m_Height;
	}

	vec2u SurfaceRGBA8_UNormImpl::size() const
	{
		return vec2( m_Width, m_Height );
	}

	uint32_t SurfaceRGBA8_UNormImpl::byte_size() const
	{
		return m_cbSize;
	}

	uint32_t SurfaceRGBA8_UNormImpl::pixel_count() const
	{
		return m_pxCount;
	}

	bool SurfaceRGBA8_UNormImpl::empty() const
	{
		return !m_pData;
	}

}
