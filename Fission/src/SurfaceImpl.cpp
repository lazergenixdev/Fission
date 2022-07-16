#include "SurfaceImpl.h"
#include <Fission/Base/Math/Vector.hpp>
#include <Fission/Base/Assert.hpp>

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
		FISSION_ASSERT( info.size.w < 16384, "Texture width too big!" );
		FISSION_ASSERT( info.size.h < 16384, "Texture height too big!" );

		if( info.size.w > 0u && info.size.h > 0u )
		{
			// width * height should never overflow, but make sure it doesn't otherwise stupid compiler won't shutup
			m_pData = std::make_unique<rgba8[]>( (size_t)info.size.w * (size_t)info.size.h );
			m_Width = info.size.w;
			m_Height = info.size.h;
			m_pxCount = m_Width * m_Height;
			m_cbSize = m_pxCount * sizeof( rgba8 );
			if( info.fillColor.has_value() )
			{
				rgba8 fill = info.fillColor.value();
				for( size_t i = 0; i < m_pxCount; i++ ) m_pData[i] = fill;
			}
		}
	}

	void SurfaceRGBA8_UNormImpl::resize( size2 _New_Size, ResizeOptions_ _Options ) {
		FISSION_THROW_NOT_IMPLEMENTED();
	}
	void SurfaceRGBA8_UNormImpl::set_width( int _New_Width, ResizeOptions_ _Options ) {
		FISSION_THROW_NOT_IMPLEMENTED();
	}
	void SurfaceRGBA8_UNormImpl::set_height( int _New_Height, ResizeOptions_ _Options ) {
		FISSION_THROW_NOT_IMPLEMENTED();
	}


	void SurfaceRGBA8_UNormImpl::insert( int _X, int _Y, PixelCallback _Source, size2 _Source_Size )
	{
		FISSION_ASSERT( _X + _Source_Size.w <= static_cast<signed int>(m_Width), "Source texture width too big!" );
		FISSION_ASSERT( _Y + _Source_Size.h <= static_cast<signed int>(m_Height), "Source texture height too big!" );

		for( int py = 0; py < _Source_Size.h; py++ )
		{
			int dst_y = py + _Y;
			rgba8 * copy_dest = &( (rgba8 *)this->m_pData.get() )[dst_y * this->m_Width + _X];

			for( int x = 0; x < _Source_Size.w; x++ )
				copy_dest[x] = _Source( x, py );
		}
	}

	void SurfaceRGBA8_UNormImpl::insert( int _X, int _Y, const Surface * _Source, std::optional<ri32> _Source_Rect )
	{
		if( _Source->format() == Texture::Format_RGBA8_UNORM )
		{
			// implement this
		}

		v2i32 start;
		size2 size;
		if( _Source_Rect.has_value() )
			size = size2(_Source_Rect->size()), start = _Source_Rect->topLeft();
		else
			size = _Source->size();

		FISSION_ASSERT( _X + size.w <= static_cast<signed int>(m_Width), "Source texture width too big!" );
		FISSION_ASSERT( _Y + size.h <= static_cast<signed int>(m_Height), "Source texture height too big!" );

		for( int py = start.y; py < size.h; py++ )
		{
			rgba8 * copy_dest = &this->m_pData[( py + _Y ) * this->m_Width + _X];

			for( int x = 0; x < size.w; x++ )
				copy_dest[x] = (rgba8)_Source->GetPixel( x + start.x, py );
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
		FISSION_ASSERT( _X < (int)m_Width, "X out of range" );
		FISSION_ASSERT( _Y < (int)m_Height, "Y out of range" );

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

	int SurfaceRGBA8_UNormImpl::width() const
	{
		return m_Width;
	}

	int SurfaceRGBA8_UNormImpl::height() const
	{
		return m_Height;
	}

	size2 SurfaceRGBA8_UNormImpl::size() const
	{
		return size2( (int)m_Width, (int)m_Height );
	}

	u64 SurfaceRGBA8_UNormImpl::byte_size() const
	{
		return m_cbSize;
	}

	u64 SurfaceRGBA8_UNormImpl::pixel_count() const
	{
		return m_pxCount;
	}

	bool SurfaceRGBA8_UNormImpl::empty() const
	{
		return !m_pData;
	}

}
