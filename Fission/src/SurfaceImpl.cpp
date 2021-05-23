#include "SurfaceImpl.h"
#include <Fission/Base/Exception.h>

namespace Fission {

	std::unique_ptr<Surface> Surface::Create( const CreateInfo & info ) {
		switch( info.format )
		{
		case Texture::Format_RGBA8_UNORM:
			return std::make_unique<SurfaceRGBA8_UNormImpl>( info );
		default: FISSION_THROW_NOT_IMPLEMENTED();
		}
	}

	SurfaceRGBA8_UNormImpl::SurfaceRGBA8_UNormImpl( const Surface::CreateInfo & info )
	{
		FISSION_ASSERT( info.width < 16384u, "Texture Width too big!" );
		FISSION_ASSERT( info.height < 16384u, "Texture Height too big!" );

		if( info.width > 0u && info.height > 0u )
		{
			// width * height should never overflow, but make sure it doesn't otherwise stupid compiler won't shutup
			m_pData = std::make_unique<rgba_color8[]>( (size_t)info.width * (size_t)info.height );
			m_Width = info.width;
			m_Height = info.height;
			m_pxCount = m_Width * m_Height;
			m_cbSize = m_pxCount * sizeof( rgba_color8 );
			if( info.fillColor.has_value() )
			{
				rgba_color8 fill = info.fillColor.value();
				for( size_t i = 0; i < m_pxCount; i++ ) m_pData[i] = fill;
			}
		}
	}

	void SurfaceRGBA8_UNormImpl::resize( vec2i _New_Size, ResizeOptions_ _Options ) {
		FISSION_THROW_NOT_IMPLEMENTED();
	}
	void SurfaceRGBA8_UNormImpl::set_width( int _New_Width, ResizeOptions_ _Options ) {
		FISSION_THROW_NOT_IMPLEMENTED();
	}
	void SurfaceRGBA8_UNormImpl::set_height( int _New_Height, ResizeOptions_ _Options ) {
		FISSION_THROW_NOT_IMPLEMENTED();
	}


	void SurfaceRGBA8_UNormImpl::insert( int _X, int _Y, PixelCallback _Source, vec2u _Source_Size )
	{
		assert( _X + _Source_Size.x <= m_Width );
		assert( _Y + _Source_Size.y <= m_Height );

		for( uint32_t py = 0; py < _Source_Size.y; py++ )
		{
			uint32_t dst_y = py + _Y;
			rgba_color8 * copy_dest = &( (rgba_color8 *)this->m_pData.get() )[dst_y * this->m_Width + _X];

			for( uint32_t x = 0; x < _Source_Size.x; x++ )
				copy_dest[x] = _Source( x, py );
		}
	}

	void SurfaceRGBA8_UNormImpl::insert( int _X, int _Y, const Surface * _Source, std::optional<recti> _Source_Rect )
	{
		if( _Source->format() == Texture::Format_RGBA8_UNORM )
		{
			// implement this
		}

		vec2u start, size;
		if( _Source_Rect.has_value() )
			size = (vec2u)_Source_Rect->size(), start = (vec2u)_Source_Rect->get_tl();
		else
			size = _Source->size();

		FISSION_ASSERT( _X + size.x <= m_Width );
		FISSION_ASSERT( _Y + size.y <= m_Height );

		for( uint32_t py = start.y; py < size.y; py++ )
		{
			rgba_color8 * copy_dest = &this->m_pData[( py + _Y ) * this->m_Width + _X];

			for( uint32_t x = 0; x < size.x; x++ )
				copy_dest[x] = (rgba_color8)_Source->GetPixel( x + start.x, py );
		}
	}

	Texture::Format SurfaceRGBA8_UNormImpl::format() const
	{
		return Texture::Format_RGBA8_UNORM;
	}

	color SurfaceRGBA8_UNormImpl::GetPixel( int _X, int _Y ) const
	{
		return (color)m_pData[_Y * m_Width + _X];
	}

	void SurfaceRGBA8_UNormImpl::PutPixel( int _X, int _Y, color _Color )
	{
		FISSION_ASSERT( _X < m_Width, "X out of range" );
		FISSION_ASSERT( _Y < m_Height, "Y out of range" );

		m_pData[_Y * m_Width + _X] = _Color;
	}

	void SurfaceRGBA8_UNormImpl::shrink_to_fit( color clear_color )
	{
		FISSION_THROW_NOT_IMPLEMENTED();
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
