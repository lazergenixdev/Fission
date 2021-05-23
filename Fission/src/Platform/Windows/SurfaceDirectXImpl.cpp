#include "../../SurfaceImpl.h"
#include <DirectXTex/DirectXTex.h>
#include <DXErr/dxerr.hpp>

namespace Fission {

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
			if( FAILED( DirectX::Convert( *image, DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, 1.0f, converted ) ) )
				return false;

			image = converted.GetImage( 0, 0, 0 );
		}

		this->m_Width = (uint32_t)image->width;
		this->m_Height = (uint32_t)image->height;
		this->m_pxCount = (uint32_t)( m_Width * m_Height );
		this->m_cbSize = (uint32_t)image->slicePitch;

		m_pData = std::make_unique<rgba_color8[]>( m_pxCount );
		memcpy( m_pData.get(), image->pixels, m_cbSize );

		return true;
	}

	bool SurfaceRGBA8_UNormImpl::Save( const file::path & _FilePath ) const
	{
		if( !m_pData ) return false;

		DirectX::Image image;
		HRESULT hr;

		image.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		image.width = (size_t)m_Width;
		image.height = (size_t)m_Height;
		image.rowPitch = (size_t)m_Width * sizeof( rgba_color8 );
		image.slicePitch = (size_t)m_cbSize;
		image.pixels = (uint8_t*)m_pData.get();

		hr = DirectX::SaveToWICFile( image, DirectX::WIC_FLAGS_NONE, GUID_ContainerFormatPng, _FilePath.c_str() );

		return FAILED( hr );
	}

}