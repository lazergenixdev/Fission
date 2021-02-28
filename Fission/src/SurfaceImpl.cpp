#include "SurfaceImpl.h"
//#include "Platform/Windows/hr_Exception.h"
#include <DirectXTex/DirectXTex.h>
#include "lazer/unfinished.h"
#include <DXErr/dxerr.hpp>

namespace Fission {

	std::unique_ptr<Surface> Surface::Create( const CreateInfo & info ) {
		switch( info.Format )
		{
		case Format::SurfaceFormat_RGBA8_UNORM:
			return std::make_unique<SurfaceRGBA8_UNormImpl>( info );
		default: _lazer_throw_not_implemented;
		}
	}

	//SurfaceRGBA::SurfaceRGBA()
	//	: m_Width( 0u ), m_Height( 0u ),
	//	m_pxCount( 0u ), m_cbSize( 0u )
	//{}

	//SurfaceRGBA::SurfaceRGBA( SurfaceRGBA & src )
	//	: m_Width( src.m_Width ), m_Height( src.m_Height ),
	//	m_pxCount( src.m_pxCount ), m_cbSize( src.m_cbSize )
	//{
	//	m_pData = std::make_unique<coloru[]>( m_pxCount );
	//	memcpy( m_pData.get(), src.m_pData.get(), m_cbSize );
	//}

	//SurfaceRGBA & SurfaceRGBA::operator=( SurfaceRGBA & src )
	//{
	//	m_Width = src.m_Width, m_Height = src.m_Height;
	//	m_pxCount = src.m_pxCount, m_cbSize = src.m_cbSize;
	//	m_pData = std::make_unique<coloru[]>( m_pxCount );
	//	memcpy( m_pData.get(), src.m_pData.get(), m_cbSize );

	//	return *this;
	//}

	//SurfaceRGBA::SurfaceRGBA( uint32_t width, uint32_t height )
	//	: m_Width( width ), m_Height( height ),
	//	m_pxCount( width * height ), m_cbSize( m_pxCount * sizeof coloru ),
	//	m_pData( std::make_unique<coloru[]>( m_pxCount ) )
	//{
	//	memset( m_pData.get(), 0, m_cbSize );
	//}

	//SurfaceRGBA::SurfaceRGBA( uint32_t width, uint32_t height, coloru fill_color )
	//	: SurfaceRGBA( width, height )
	//{
	//	if( fill_color.r == fill_color.g && fill_color.r == fill_color.b && fill_color.r == fill_color.a )
	//		memset( m_pData.get(), fill_color.r, m_cbSize );
	//	else for( uint32_t i = 0; i < m_pxCount; i++ )
	//			m_pData[i] = fill_color;
	//}

	//bool SurfaceRGBA::Load( const wchar_t * filename )
	//{
	//	HRESULT hr;
	//	DirectX::ScratchImage scratch;
	//	DirectX::ScratchImage converted;

	//	if( FAILED( hr = DirectX::LoadFromWICFile( filename, DirectX::WIC_FLAGS_NONE, nullptr, scratch ) ) )
	//		throw hr_Exception( hr, __LINE__, __FILEW__ ); // TODO: create surface exceptions

	//	auto image = scratch.GetImage( 0, 0, 0 );
	//	/* Image in an incorrect format */
	//	if( image->format != DXGI_FORMAT_R8G8B8A8_UNORM )
	//	{
	//		 but realistically we should not be taking any computation to convert, LOL
	//		throw std::logic_error("Loaded texture has an incorrect format");

	//		if( FAILED( DirectX::Convert( *image, DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, 1.0f, converted ) ) )
	//			return false;
	//		image = converted.GetImage( 0, 0, 0 );
	//	}

	//	this->m_Width = image->width;
	//	this->m_Height = image->height;
	//	this->m_pxCount = m_Width * m_Height;
	//	this->m_cbSize = image->slicePitch;

	//	m_pData = std::make_unique<coloru[]>( m_pxCount );
	//	memcpy( m_pData.get(), image->pixels, m_cbSize );

	//	return true;
	//}

	//bool SurfaceRGBA::Save( const wchar_t * filename )
	//{
	//	GUID file_format_guid;

	//	file_format_guid = DirectX::GetWICCodec( DirectX::WIC_CODEC_PNG );

	//	DirectX::Image image;
	//	image.pixels = (uint8_t*)m_pData.get();
	//	image.width = m_Width;
	//	image.height = m_Height;
	//	image.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//	image.rowPitch = m_Width * sizeof coloru;
	//	image.slicePitch = m_cbSize;

	//	DirectX::SaveToWICFile( image, DirectX::WIC_FLAGS_NONE, file_format_guid, filename );
	//	return false;
	//}

	//void SurfaceRGBA::SetWidth( uint32_t new_width )
	//{
	//	assert( new_width > 0u );

	//	SurfaceRGBA _new( new_width, m_Height );

	//	auto pData = std::make_unique<coloru[]>( new_width * m_Height );

	//	const uint32_t width = std::min( new_width, m_Width );

	//	for( uint32_t y = 0; y < m_Height; y++ )
	//		for( uint32_t x = 0; x < width; x++ )
	//			pData[y * new_width + x] = m_pData[y * m_Width + x];

	//	m_Width = new_width;
	//	m_pData.reset( pData.release() );
	//}

	//void SurfaceRGBA::SetHeight( uint32_t new_height )
	//{
	//	assert( new_height > 0u );

	//	SurfaceRGBA _new( m_Width, new_height );

	//	const int height = std::min( new_height, m_Height );

	//	for( int y = 0; y < height; y++ )
	//		for( int x = 0; x < m_Width; x++ )
	//			_new[y * m_Width + x] = m_pData[y * m_Width + x];

	//	*this = _new;
	//}

	//void SurfaceRGBA::Insert( uint32_t x, uint32_t y, PixelCallbackRGBA src, vec2i src_size )
	//{
	//	assert( x + src_size.x <= m_Width );
	//	assert( y + src_size.y <= m_Height );

	//	for( int py = 0; py < src_size.y; py++ )
	//	{
	//		int dst_y = py + y;
	//		coloru * copy_dest = &this->m_pData[dst_y * this->m_Width + x];

	//		for( int x = 0; x < src_size.x; x++ )
	//			copy_dest[x] = src( x, py );
	//	}
	//}

	SurfaceRGBA8_UNormImpl::SurfaceRGBA8_UNormImpl( const Surface::CreateInfo & info )
	{
		if( info.Width > 0u && info.Height > 0u )
		{
			m_pData = std::make_unique<coloru[]>( info.Width * info.Height );
			m_Width = info.Width;
			m_Height = info.Height;
		}
	}

	bool SurfaceRGBA8_UNormImpl::Load( const file::path & _FilePath )
	{
		HRESULT hr;
		DirectX::ScratchImage scratch;
		DirectX::ScratchImage converted;

		if( FAILED( hr = DirectX::LoadFromWICFile( _FilePath.wstring().c_str(), DirectX::WIC_FLAGS_NONE, nullptr, scratch ) ) )
		{
			static char desc[512];
			DXGetErrorDescriptionA( hr, desc, std::size( desc ) );
			throw lazer::exception( "Surface Exception", _lazer_exception_msg.append( "HRESULT", desc ) );
			return false;
		}

		auto image = scratch.GetImage( 0, 0, 0 );
		/* Image in an incorrect format */
		if( image->format != DXGI_FORMAT_R8G8B8A8_UNORM )
		{
			// but realistically we should not be taking any computation to convert, LOL
			//throw std::logic_error("Loaded texture has an incorrect format");

			if( FAILED( DirectX::Convert( *image, DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, 1.0f, converted ) ) )
				return false;

			image = converted.GetImage( 0, 0, 0 );
		}

		this->m_Width = (uint32)image->width;
		this->m_Height = (uint32)image->height;
		this->m_pxCount = (uint32)(m_Width * m_Height);
		this->m_cbSize = (uint32)image->slicePitch;

		m_pData = std::make_unique<coloru[]>( m_pxCount );
		memcpy( m_pData.get(), image->pixels, m_cbSize );

		return true;
	}

	bool SurfaceRGBA8_UNormImpl::Save( const file::path & _FilePath )
	{
		return false;
	}

	void SurfaceRGBA8_UNormImpl::resize( vec2 new_size, ResizeOptions_ options )
	{
		_lazer_throw_not_implemented;
	}

	void SurfaceRGBA8_UNormImpl::set_width( uint32 new_width, ResizeOptions_ options )
	{
		_lazer_throw_not_implemented;
	}

	void SurfaceRGBA8_UNormImpl::set_height( uint32 new_height, ResizeOptions_ options )
	{
		_lazer_throw_not_implemented;
	}

	void SurfaceRGBA8_UNormImpl::insert( int x, int y, PixelCallback src, vec2i src_size )
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

	Texture::Format SurfaceRGBA8_UNormImpl::format() const
	{
		return Texture::Format_RGBA8_UNORM;
	}

	void SurfaceRGBA8_UNormImpl::PutPixel( uint32 x, uint32 y, color color )
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

	uint32 SurfaceRGBA8_UNormImpl::width() const
	{
		return m_Width;
	}

	uint32 SurfaceRGBA8_UNormImpl::height() const
	{
		return m_Height;
	}

	Surface::vec2 SurfaceRGBA8_UNormImpl::size() const
	{
		return vec2( m_Width, m_Height );
	}

	uint32 SurfaceRGBA8_UNormImpl::byte_size() const
	{
		return m_cbSize;
	}

	uint32 SurfaceRGBA8_UNormImpl::pixel_count() const
	{
		return m_pxCount;
	}

	bool SurfaceRGBA8_UNormImpl::empty() const
	{
		return !m_pData;
	}

}
